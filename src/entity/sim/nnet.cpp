//
//  entity/sim/nnet.cpp
//  crag
//
//  Created by John McFarlane on 2015-06-09.
//  Copyright 2015 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "nnet.h"

#include "sim/Engine.h"

#include <core/RosterObjectDefine.h>

using namespace sim::nnet;

////////////////////////////////////////////////////////////////////////////////
// sim::nnet::Input

CRAG_ROSTER_OBJECT_DEFINE(
	Input,
	1200,
	Pool::CallBefore<& Input::Tick, Neuron, & Neuron::Tick>(Engine::GetTickRoster()));

void Input::Connect(Connection const & connection) noexcept
{
	CRAG_VERIFY_TRUE(std::none_of(std::begin(connections), std::end(connections), [connection] (Connection c)
	{
		return connection.neuron == c.neuron;
	}));

	connections.push_back(connection);
}

void Input::Tick() noexcept
{
	auto s = GetSignal();
	for (auto connection : connections)
	{
		connection.neuron->Add(connection.weight * s);
	}
}

////////////////////////////////////////////////////////////////////////////////
// sim::nnet::Neuron

CRAG_ROSTER_OBJECT_DEFINE(
	Neuron,
	2000,
	Pool::Call<& Neuron::Tick>(Engine::GetTickRoster()));

Neuron::Neuron(ga::GenomeReader & genome_reader) noexcept
	: sigmoid(genome_reader.Read(), genome_reader.Read(), genome_reader.Read(), genome_reader.Read())
{
}

void Neuron::Add(SignalType s) noexcept
{
	sum += s;
}

void Neuron::Tick() noexcept
{
	TransmitSignal(sigmoid(sum));
	sum = 0;
}

////////////////////////////////////////////////////////////////////////////////
// sim::nnet::Layer

Layer::Layer(ga::GenomeReader & genome_reader, int num_inputs, int num_neurons) noexcept
{
	neurons.reserve(num_neurons);
	for (auto count = num_neurons; count; -- count)
	{
		neurons.emplace_back(new Neuron(genome_reader));
	}

	inputs.reserve(num_inputs);
	for (auto count = num_inputs; count; -- count)
	{
		auto input = std::unique_ptr<Input>(new Input);

		for (auto & neuron_ptr : neurons)
		{
			input->Connect(Connection{
				neuron_ptr.get(),
				genome_reader.Read() * 2.f - 1.f
			});
		}

		inputs.push_back(std::move(input));
	}
}

void Layer::Connect(Layer & output, Layer & input) noexcept
{
	CRAG_VERIFY_EQUAL(output.neurons.size(), input.inputs.size());

	for (auto i = std::make_pair(std::begin(output.neurons), std::begin(input.inputs));
		i.first != std::end(output.neurons);
		++ i.second, ++ i.first)
	{
		CRAG_VERIFY_TRUE(i.second != std::end(input.inputs));

		(* i.first)->SetReceiver(i.second->get());
	}
}

void Layer::ConnectInputs(std::vector<Transmitter *> const & transmitters) noexcept
{
	CRAG_VERIFY_EQUAL(inputs.size(), transmitters.size());

	for (
		auto iterators = std::make_pair(std::begin(transmitters), std::begin(inputs));
		iterators.first != std::end(transmitters);
		++ iterators.first, ++ iterators.second)
	{
		(* iterators.first)->SetReceiver(iterators.second->get());
	}
}

void Layer::ConnectOutputs(std::vector<Receiver *> const & receivers) noexcept
{
	CRAG_VERIFY_EQUAL(neurons.size(), receivers.size());

	for (
		auto iterators = std::make_pair(std::begin(neurons), std::begin(receivers));
		iterators.first != std::end(neurons);
		++ iterators.first, ++ iterators.second)
	{
		(* iterators.first)->SetReceiver(* iterators.second);
	}
}

////////////////////////////////////////////////////////////////////////////////
// sim::nnet::Network

Network::Network(ga::Genome & genome, std::vector<int> const & num_layer_nodes) noexcept
{
	ga::GenomeReader genome_reader(genome);

	auto num_layers = num_layer_nodes.size() - 1;
	layers.reserve(num_layers);

	for (auto i = 0u; i != num_layers; ++ i)
	{
		auto num_inputs = num_layer_nodes[i];
		auto num_outputs = num_layer_nodes[i + 1];

		layers.emplace_back(genome_reader, num_inputs, num_outputs);
	}

	for (auto layer_index0 = 0u, layer_index1 = 1u; layer_index1 != num_layers; ++ layer_index0, ++ layer_index1)
	{
		auto & output_layer = layers[layer_index0];
		auto & input_layer = layers[layer_index1];

		Layer::Connect(output_layer, input_layer);
	}
}

void Network::ConnectInputs(std::vector<Transmitter *> const & transmitters) noexcept
{
	layers.front().ConnectInputs(transmitters);
}

void Network::ConnectOutputs(std::vector<Receiver *> const & receivers) noexcept
{
	layers.back().ConnectOutputs(receivers);
}
