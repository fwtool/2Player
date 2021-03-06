
#ifndef __SSet_H
#define __SSet_H

namespace NS_SSTL
{
#undef __Super__
#define __Super__ SContainerT<__BaseType<__DataType>>

	template<typename __DataType, template<typename...> class __BaseType>
	class SSetT : public __Super__
	{
	private:
		__UsingSuper(__Super__);
		
	public:
		SSetT() = default;

		template<typename... args>
		explicit SSetT(__DataConstRef data, const args&... others)
			: __Super(data, others...)
		{
		}

		explicit SSetT(__ContainerType&& container)
		{
			__Super::swap(container);
		}

		SSetT(SSetT&& set)
		{
			__Super::swap(set);
		}

		SSetT(const SSetT& set)
			: __Super(set)
		{
		}

		explicit SSetT(__InitList initList)
			: __Super(initList)
		{
		}

		template<typename T, typename = checkContainer_t<T>>
		explicit SSetT(const T& container)
			: __Super(container)
		{
		}
		
		SSetT& operator=(__ContainerType&& container)
		{
			__Super::swap(container);
			return *this;
		}
		
		SSetT& operator=(SSetT&& set)
		{
			__Super::swap(set);
			return *this;
		}

		SSetT& operator=(const SSetT& set)
		{
			__Super::assign(set);
			return *this;
		}

		SSetT& operator=(__InitList initList)
		{
			__Super::assign(initList);
			return *this;
		}

		template <typename T>
		SSetT& operator=(const T&t)
		{
			__Super::assign(t);
			return *this;
		}

		//template <typename T>
		//SSetT& operator=(T&t)
		//{
		//	__Super::assign(t);
		//	return *this;
		//}

	public:
		template <typename T>
		SSetT<T, __BaseType> map(CB_T_Ret<__DataConstRef, T> cb) const
		{
			SSetT<T, __BaseType> set;

			for (auto&data : m_data)
			{
				set.add(cb(data));
			}
			
			return set;
		}

		template <typename CB, typename RET = decltype(declval<CB>()(__DataType()))>
		SSetT<RET, __BaseType> map(const CB& cb) const
		{
			return map<RET>(cb);
		}

	private:
		void _add(__DataConstRef data) override
		{
			m_data.insert(data);
		}

		size_t _del(__DataConstRef data, CB_Del cb) override
		{
			auto itr = m_data.find(data);
			if (itr == m_data.end())
			{
				return 0;
			}

			size_t uRet = 0;
			do
			{
				E_DelConfirm eRet = cb(*itr);
				if (E_DelConfirm::DC_Abort == eRet)
				{
					break;
				}
				else if (E_DelConfirm::DC_No == eRet)
				{
					++itr;
				}
				else
				{
					itr = m_data.erase(itr);
					uRet++;

					if (E_DelConfirm::DC_YesAbort == eRet)
					{
						break;
					}
				}
			} while (itr != m_data.end() && *itr == data);

			return uRet;
		}

		bool _includes(__DataConstRef data) const override
		{
			return m_data.find(data) != m_data.end();
		}
	};
}

#endif // __SSet_H
