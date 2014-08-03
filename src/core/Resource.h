//
//  core/Resource.h
//  crag
//
//  Created by John McFarlane on 2014-01-18.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "TypeId.h"

namespace crag
{
	namespace core
	{
		// forward-declarations
		template <typename Type>
		class ResourceHandle;
		class WrapperInterface;
		
		// stub for an object which can be retrieved by name from ResourceManager;
		// object can be destroyed and recreated without Resource object moving
		class Resource
		{
			// friends
			template <typename Type>
			friend class ResourceHandle;

			////////////////////////////////////////////////////////////////////////////////
			// types
			
			// WrapperInterface and Wrapper hold the actual resource and can be dynamically
			// destroyed and recreated using the create function if memory is tight
			class WrapperInterface
			{
				OBJECT_NO_COPY(WrapperInterface);
			public:
				WrapperInterface() = default;
				virtual ~WrapperInterface() = default;
				virtual void const * get() const = 0;
			};

			using WrapperUniquePtr = std::unique_ptr<WrapperInterface>;
			using CreateFunctionType = WrapperInterface * ();
			using CreateFunctionWrapperType = std::function<CreateFunctionType>;
			
		public:
			template <typename Type>
			class Wrapper final : public WrapperInterface
			{
				OBJECT_NO_COPY(Wrapper);
			public:
				Wrapper(Type && value) 
				: _value(std::move(value)) 
				{
				}

				~Wrapper() noexcept
				{
				}
			private:
				void const * get() const override
				{ 
					return & _value; 
				}

				Type _value;
			};

			////////////////////////////////////////////////////////////////////////////////
			// functions

			OBJECT_NO_COPY(Resource);
			CRAG_VERIFY_INVARIANTS_DECLARE(Resource);
			
			Resource(CreateFunctionWrapperType create_function, TypeId type_id);
			Resource(Resource && rhs);
			
			template <typename Type, typename Function>
			static Resource Create(Function function);
			
			template <typename Type>
			Type const & get() const;
			
			void Prefetch() const;
			void Flush() const;
			
			TypeId GetTypeId() const;
			
			////////////////////////////////////////////////////////////////////////////////
			// variables

		private:
			mutable WrapperUniquePtr _object;
			CreateFunctionWrapperType _create_function;
			TypeId _type_id;
		};

		////////////////////////////////////////////////////////////////////////////////
		// crag::core::Resource member template definitions

		template <typename Type, typename Function>
		Resource Resource::Create(Function function)
		{
			using FunctionType = std::function<Function>;
			static_assert(std::is_same<typename FunctionType::result_type, Type>::value, "Function does not return given Type");
			
			return Resource([function] () { return function(); }, TypeId::Create<Type>());
		}
		
		template <typename Type>
		Type const & Resource::get() const
		{
			Prefetch();
			
			auto const resource_wrapper = _object.get();
			ASSERT(resource_wrapper);
			
			auto const object = resource_wrapper->get();
			ASSERT(object);
			
			return * reinterpret_cast<Type const *>(object);
		}
	}
}
