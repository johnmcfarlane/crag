//
//  entity/sim/nnet.h
//  crag
//
//  Created by John McFarlane on 2015-06-09.
//  Copyright 2015 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Signal.h"
#include "Genome.h"

#include <core/RosterObjectDeclare.h>
#include <core/Sigmoid.h>

namespace sim
{
	namespace nnet
	{
		class Neuron;

		struct Connection final
		{
			constexpr Connection() = default;
			constexpr Connection(Neuron * n, SignalType w) noexcept : neuron(n), weight(w) { }

			Neuron * neuron = nullptr;
			SignalType weight = 0;
		};

		class Input final
			: public Receiver
		{
		public:
			// functions
			CRAG_ROSTER_OBJECT_DECLARE(Input);

			Input() = default;

			void Connect(Connection const & connection) noexcept;

			void Tick() noexcept;

		private:
			// variables
			std::vector<Connection> connections;
		};

		class Neuron final
			: public Transmitter
		{
		public:
			// types
			using Sigmoid = crag::core::Sigmoid<SignalType>;

			// functions
			CRAG_ROSTER_OBJECT_DECLARE(Neuron);

			Neuron(ga::GenomeReader & genome_reader) noexcept;

			void Add(SignalType s) noexcept;

			void Tick() noexcept;

		private:
			// variables
			SignalType sum = 0;
			Sigmoid sigmoid;
		};

		class Layer final
		{
		public:
			// functions
			OBJECT_NO_COPY(Layer);

			Layer(ga::GenomeReader & genome_reader, int num_inputs, int num_neurons) noexcept;
			Layer(Layer && rhs) noexcept;

			// connect together the neighbouring layers in a feed-forward network
			static void Connect(Layer & output, Layer & intput) noexcept;

			void ConnectInputs(std::vector<Transmitter *> const & transmitters) noexcept;
			void ConnectOutputs(std::vector<Receiver *> const & receivers) noexcept;

		private:
			// variables
			std::vector<std::unique_ptr<Input>> inputs;
			std::vector<std::unique_ptr<Neuron>> neurons;
		};

		class Network final
		{
		public:
			// functions
			Network() = default;
			Network(Network &&) = default;
			Network(ga::Genome & genome, std::vector<int> const & num_layer_nodes) noexcept;

			Network & operator=(Network &&) = default;

			void ConnectInputs(std::vector<Transmitter *> const & transmitters) noexcept;
			void ConnectOutputs(std::vector<Receiver *> const & receivers) noexcept;

		private:
			// variables
			std::vector<Layer> layers;
		};
	}
}
