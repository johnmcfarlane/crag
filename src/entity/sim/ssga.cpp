//
//  ssga.cpp
//  crag
//
//  Created by John McFarlane on 2015-07-05.
//  Copyright 2015 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "ssga.h"

#include "AnimatController.h"
#include "Genome.h"
#include "Health.h"

#include <entity/physics/AnimatBody.h>
#include <entity/sim/AnimatModel.h>

#include <sim/Engine.h>
#include <sim/gravity.h>

#include <physics/Engine.h>

#include <form/RayCastResult.h>

#include <gfx/Engine.h>
#include <gfx/object/Ball.h>

#include <core/ConfigEntry.h>
#include <core/iterable_object_pool.h>
#include <core/Random.h>

using namespace sim;
using ga::Genome;
using ga::Gene;

namespace
{
	////////////////////////////////////////////////////////////////////////////////
	// constants

	CONFIG_DEFINE(max_num_animats, 25);

	CONFIG_DEFINE(animat_radius, 1.f);
	CONFIG_DEFINE(animat_birth_elevation, 5.f);
	CONFIG_DEFINE(animat_elevation_test_length, 10000.f);
	CONFIG_DEFINE(animat_start_distribution, 25.f);

	geom::abs::Vector3 animat_start_pos;
	constexpr auto ga_filename = "ga.csv";

	////////////////////////////////////////////////////////////////////////////////
	// functions

	// given horizontal_position within search_radius distance of formation surface,
	// return surface ray
	Ray3 GetSurface(Engine & engine, Vector3 const & horizontal_position, Scalar search_radius)
	{
		// determine which way is down at the given position
		auto gravity = GetGravitationalForce(engine, horizontal_position);
		auto down_direction = geom::Normalized(gravity);

		// create a downward-pointing unit ray which points through the given position from above
		auto cast_ray = Ray3(horizontal_position - down_direction * search_radius, down_direction);

		// where does ray hit a surface?
		auto & physics_engine = engine.GetPhysicsEngine();
		auto search_diameter = 2.f * search_radius;
		auto result = physics_engine.CastRay(cast_ray, search_diameter);

		if (! result)
		{
			DEBUG_BREAK("CastRay returned no result");

			// of any point on the ray that was tested,
			// this presumably has the most chance if being in the clear
			return Ray3(cast_ray.position, - cast_ray.direction);
		}

		return Ray3(
			geom::Project(cast_ray, result.GetDistance()),
			result.GetNormal());
	}

	// given a horizontal_position with altitude (- search_radius, search_radius),
	// return a position that is given distance from ground at that position
	Vector3 GetElevation(Engine & engine, Vector3 const & horizontal_position, Scalar height, Scalar search_radius)
	{
		auto surface = GetSurface(engine, horizontal_position, search_radius);

		// actually in the direction of the surface normal; not the upward direction
		return geom::Project(surface, height);
	}

	Vector3 GetAnimatSpawnPosition(Engine & engine) noexcept
	{
		auto const & space = engine.GetSpace();
		auto base_position = space.AbsToRel(animat_start_pos);

		Vector3 offset;
		float r;
		Random::sequence.GetGaussians(offset.x, offset.y);
		offset.y = std::abs(offset.y);
		Random::sequence.GetGaussians(offset.z, r);

		auto horizontal_position = base_position + offset * animat_start_distribution;

		auto position = GetElevation(engine, horizontal_position, animat_birth_elevation, animat_elevation_test_length);
		return position;
	}

	// create an animat entity near given horizontal_position with given genome
	void CreateAnimat(Engine & engine, Genome && genome) noexcept
	{
		auto handle = EntityHandle::CreateFromUid(ipc::Uid::Create());
		auto entity = engine.CreateObject<Entity>(handle);

		// health
		auto health = new Health(entity->GetHandle());

		// physics
		auto spawn_position = GetAnimatSpawnPosition(engine);
		Sphere3 sphere(spawn_position, animat_radius);
		physics::Engine & physics_engine = engine.GetPhysicsEngine();
		auto body = new physics::AnimatBody(
			sphere.center, & Vector3::Zero(), physics_engine,
			sphere.radius, * health);
		body->SetDensity(1);
		entity->SetLocation(std::unique_ptr<physics::Location>(body));

		// graphics
		gfx::Transformation local_transformation(sphere.center, gfx::Transformation::Matrix33::Identity(), sphere.radius);
		gfx::ObjectHandle model_handle = gfx::BallHandle::Create(local_transformation, sphere.radius, gfx::Color4f::Green());
		auto model = new AnimatModel(model_handle, * body);
		entity->SetModel(Entity::ModelPtr(model));

		// controller
		auto controller = new AnimatController(
			* entity, sphere.radius, std::move(genome), AnimatController::HealthPtr(health));
		entity->SetController(std::unique_ptr<AnimatController>(controller));

		// connect health signal
		health->GetTransmitter().AddReceiver(model->GetHealthReceiver());
	}

	// create a semi-randomly placed population of animats with given genomes, gene_pool
	void GeneratePopulation(Engine & engine, std::vector<Genome> & gene_pool) noexcept
	{
		for (auto & genome : gene_pool)
		{
			CreateAnimat(engine, std::move(genome));
		}
	}

	// call GeneratePopulation with gene_pool loaded from file with given filename
	std::vector<Genome> LoadGenePool(char const * filename) noexcept
	{
		using PopulationType = std::vector<Genome>;
		PopulationType population;

		auto file = std::fopen(app::GetStatePath(filename).c_str(), "r");
		if (! file)
		{
			return population;
		}

		Genome genome;
		for (;;)
		{
			CRAG_VERIFY_TRUE(std::feof(file) == 0);

			static_assert(std::is_same<Gene::unit_repr_type, std::uint32_t>::value, "bad assumption about Gene storage type");
			Gene::unit_repr_type repr;
			char delimiter;

			int num_fields_scanned = fscanf(file, "%" PRIu32 "%c", & repr, & delimiter);
			switch (num_fields_scanned)
			{
				case 2:
					break;

				case EOF:
					return population;

				default:
					ERROR_MESSAGE("%d fields of \"%s\" scanned", num_fields_scanned, filename);
					return PopulationType();
			}

			genome.push_back(Gene::from_repr(repr));

			switch (delimiter)
			{
				case ',':
					break;

				case '\n':
					population.push_back(std::move(genome));
					break;

				default:
					ERROR_MESSAGE(
						"unrecognized delimeter, %d, on line %d of %s",
						delimiter, static_cast<int>(population.size()) + 1, filename);
					return PopulationType();
			}
		}
	}

	// dump all genomes from AnimatController instances to file with given filename
	void SavePopulation(char const * filename) noexcept
	{
		auto file = std::fopen(app::GetStatePath(filename).c_str(), "w");
		if (! file)
		{
			ERROR_MESSAGE("failed to open \"%s\" for writing", filename);
			return;
		}

		auto & pool = AnimatController::GetPool();
		pool.for_each([file] (AnimatController const & animat_controller) {
			auto const & genome = animat_controller.GetGenome();

			bool first = true;
			for (auto gene : genome)
			{
				if (first)
				{
					first = false;
				}
				else
				{
					std::fputc(',', file);
				}

				auto repr = gene.to_repr();
				std::fprintf(file, "%" PRIu32, repr);
			}

			std::fputc('\n', file);
		});

		std::fclose(file);
	}

	// call GeneratePopulation with gene_pool loaded from file with given filename
	// or return false
	bool LoadPopulation(Engine & engine, char const * filename) noexcept
	{
		auto population = LoadGenePool(filename);

		if (population.empty())
		{
			return false;
		}
		CRAG_VERIFY_EQUAL(population.size(), std::size_t(max_num_animats));

		GeneratePopulation(engine, population);
		return true;
	}

	// spawn a new animat from existing population
	void Breed(Engine & engine) noexcept
	{
		auto & pool = AnimatController::GetPool();
		auto pool_size = pool.size();

		auto parent_genomes = std::array<Genome const *, 2>();
		auto sum_position = Vector3::Zero();

		for (auto parent_index = 0; parent_index != 2; ++ parent_index)
		{
			// get controller
			auto pool_index = Random::sequence.GetInt(pool_size);

			auto parent_controller = static_cast<AnimatController * const>(nullptr);
			pool.for_each([& pool_index, & parent_controller] (AnimatController & controller) {
				if (! pool_index)
				{
					parent_controller = & controller;
				}

				-- pool_index;
			});
			CRAG_VERIFY_TRUE(parent_controller);

			// get genome
			parent_genomes[parent_index] = & parent_controller->GetGenome();

			// positional
			auto & location = * parent_controller->GetEntity().GetLocation();
			sum_position += location.GetTranslation();
		};

		auto child_genome = Genome(* parent_genomes[0], * parent_genomes[1]);

		CreateAnimat(engine, std::move(child_genome));
	}
}

namespace ssga
{
	void Init(Engine & engine, geom::abs::Vector3 const & spawn_pos) noexcept
	{
		animat_start_pos = spawn_pos;
		if (! LoadPopulation(engine, ga_filename))
		{
			auto gene_pool = std::vector<Genome>(max_num_animats);
			GeneratePopulation(engine, gene_pool);
		}
	}

	void Deinit(Engine &) noexcept
	{
		SavePopulation(ga_filename);
	}

	void Tick(Engine & engine) noexcept
	{
		auto & pool = AnimatController::GetPool();

		auto num_animats = pool.size();
		CRAG_VERIFY_OP(num_animats, >=, 0);
		CRAG_VERIFY_OP(num_animats, <=, max_num_animats);

		if (num_animats == 0)
		{
			ERROR_MESSAGE("There are no animats! Regenerating...");
			Init(engine, animat_start_pos);
		}
		else if (num_animats < max_num_animats)
		{
			while (pool.size() < max_num_animats)
			{
				Breed(engine);
			}

			SavePopulation(ga_filename);
		}

		CRAG_VERIFY_EQUAL(pool.size(), max_num_animats);
	}
}
