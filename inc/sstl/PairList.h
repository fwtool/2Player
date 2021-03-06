
#ifndef __PairList_H
#define __PairList_H

namespace NS_SSTL
{
#define __Pair__ pair<__FirstType, __SecondType>
#undef __Super__
#define __Super__ ArrListT<pair<__FirstType, __SecondType>>

	template <typename __FirstType, typename __SecondType>
	class PairListT : public __Super__
	{
	private:
		__UsingSuper(__Super__)

		using __PairType = __Pair__;
		using __PairRef = __PairType&;
		using __PairConstRef = const __PairType&;
		
		using __AdpPairType = pair<__FirstType const, __SecondType>;
		
		using __FirstRef = __FirstType&;
		using __FirstConstRef = const __FirstType&;

		using __SecondRef = __SecondType&;
		using __SecondConstRef = const __SecondType&;

		using __CB_FirstRef_void = CB_T_void<__FirstRef>;
		using __CB_FirstRef_bool = CB_T_bool<__FirstRef>;

		using __CB_FirstConstRef_void = CB_T_void<__FirstConstRef>;
		using __CB_FirstConstRef_bool = CB_T_bool<__FirstConstRef>;

		using __CB_SecondRef_void = CB_T_void<__SecondRef>;
		using __CB_SecondRef_bool = CB_T_bool<__SecondRef>;

		using __CB_SecondConstRef_void = CB_T_void<__SecondConstRef>;
		using __CB_SecondConstRef_bool = CB_T_bool<__SecondConstRef>;

	public:
		PairListT() = default;
		
		explicit PairListT(__FirstConstRef first, __SecondConstRef second)
		{
			_add({ first, second });
		}
		
		template<typename... args>
		explicit PairListT(__DataConstRef data, const args&... others)
			: __Super(data, others...)
		{
		}

		explicit PairListT(__ContainerType&& container)
		{
			__Super::swap(container);
		}

		PairListT(PairListT&& lst)
		{
			__Super::swap(lst);
		}

		PairListT(const PairListT& lst)
			: __Super(lst)
		{
		}

		explicit PairListT(__InitList initList)
			: __Super(initList)
		{
		}

		template<typename T, typename = checkContainerData_t<T, __PairType>>
		explicit PairListT(const T& container)
			: __Super(container)
		{
		}

		template<typename T, typename = checkContainerData_t<T, __AdpPairType>, typename = void>
		explicit PairListT(const T& map)
		{
			for (auto& pr : map)
			{
				__Super::_add(pr);
			}
		}

		PairListT& operator=(__ContainerType&& container)
		{
			__Super::swap(container);
			return *this;
		}

		PairListT& operator=(PairListT&& lst)
		{
			__Super::swap(lst);
			return *this;
		}

		PairListT& operator=(const PairListT& lst)
		{
			__Super::assign(lst);
			return *this;
		}

		PairListT& operator=(__InitList initList)
		{
			__Super::assign(initList);
			return *this;
		}

		template<typename T>
		PairListT& operator=(const T&t)
		{
			__Super::assign(t);
			return *this;
		}

		template<typename T, typename = checkContainerData_t<T, __AdpPairType>>
		PairListT& operator=(const T&map)
		{
			__Super::clear();
			for (auto& pr : map)
			{
				__Super::_add(pr);
			}

			return *this;
		}

		PairListT& operator=(const __AdpPairType& pr)
		{
			__Super::assign((__PairConstRef)pr);
			return *this;
		}

		template <typename CB>
		void operator() (int startPos, int endPos, const CB& cb)
		{
			adaptor().forEach(cb, startPos, endPos);
		}

		template <typename CB>
		void operator() (int startPos, int endPos, const CB& cb) const
		{
			adaptor().forEach(cb, startPos, endPos);
		}

		template <typename CB>
		void operator() (int startPos, const CB& cb)
		{
			adaptor().forEach(cb, startPos);
		}

		template <typename CB>
		void operator() (int startPos, const CB& cb) const
		{
			adaptor().forEach(cb, startPos);
		}

		template <typename CB>
		void operator() (const CB& cb)
		{
			adaptor().forEach(cb);
		}

		template <typename CB>
		void operator() (const CB& cb) const
		{
			adaptor().forEach(cb);
		}

	public:
		bool getFirst(size_t pos, __CB_FirstRef_void cb)
		{
			return __Super::get(pos, [&](__PairRef pr) {
				cb(pr.first);
			});
		}

		bool getFirst(size_t pos, __CB_FirstConstRef_void cb) const
		{
			return __Super::get(pos, [&](__PairConstRef pr) {
				cb(pr.first);
			});
		}

		bool getFirst(size_t pos, __FirstRef& first) const
		{
			return __Super::get(pos, [&](__PairConstRef pr) {
				first = pr.first;
			});
		}

		bool getSecond(size_t pos, __CB_SecondRef_void cb)
		{
			return __Super::get(pos, [&](__PairRef pr) {
				cb(pr.second);
			});
		}

		bool getSecond(size_t pos, __CB_SecondConstRef_void cb) const
		{
			return __Super::get(pos, [&](__PairConstRef& pr) {
				cb(pr.second);
			});
		}

		bool getSecond(size_t pos, __SecondRef& first) const
		{
			return __Super::get(pos, [&](__PairRef pr) {
				first = pr.second;
			});
		}

		bool setFirst(size_t pos, __FirstConstRef first)
		{
			return __Super::get(pos, [&](__PairRef pr) {
				pr.first = first;
			});
		}

		bool setSecond(size_t pos, __SecondConstRef second)
		{
			return __Super::get(pos, [&](__PairRef pr) {
				pr.second = second;
			});
		}

		bool set(size_t pos, __PairConstRef pr)
		{
			return __Super::set(pos, pr);
		}

		bool set(size_t pos, __FirstConstRef first, __SecondConstRef second)
		{
			return __Super::set(pos, { first, second });
		}

		template <typename CB>
		void forFirst(const CB& cb, int startPos = 0, int endPos = -1)
		{
			adaptor().forFirst(cb, startPos, endPos);
		}

		template <typename CB>
		void forFirst(const CB& cb, int startPos = 0, int endPos = -1) const
		{
			adaptor().forFirst(cb, startPos, endPos);
		}

		template <typename CB>
		void forSecond(const CB& cb, int startPos = 0, int endPos = -1)
		{
			adaptor().forSecond(cb, startPos, endPos);
		}

		template <typename CB>
		void forSecond(const CB& cb, int startPos = 0, int endPos = -1) const
		{
			adaptor().forSecond(cb, startPos, endPos);
		}

		inline PairListT& addPair(__FirstConstRef first, __SecondConstRef second)
		{
			__Super::_add({ first, second });
			return *this;
		}
		
	public:
		template <typename CB>
		SArray<__FirstType> firsts(const CB& cb) const
		{
			return adaptor().firsts(cb);
		}

		SArray<__FirstType> firsts() const
		{
			return adaptor().firsts([&](__FirstConstRef) {
				return true;
			});
		}

		template <typename CB>
		SArray<__SecondType> seconds(const CB& cb) const
		{
			return adaptor().seconds(cb);
		}

		SArray<__SecondType> seconds() const
		{
			return adaptor().seconds([&](__SecondConstRef) {
				return true;
			});
		}

        template <typename CB, typename RET = SMap<cbRet_t<CB, __FirstConstRef>, __SecondType> >
		RET mapFirst(const CB& cb) const
		{
			RET lst;

			for (auto& pr : m_data)
			{
				lst.addPair(cb(pr.first), pr.second);
			}

			return lst;
		}

        template <typename CB, typename RET = SMap<__FirstType, cbRet_t<CB, __SecondConstRef>> >
		RET mapSecond(const CB& cb) const
		{
			RET lst;

			for (auto& pr : m_data)
			{
				lst.addPair(pr.first, cb(pr.second));
			}

			return lst;
		}

		template <typename CB, typename = checkCBBool_t<CB, __FirstConstRef, __SecondConstRef> >
		PairListT filter(const CB& cb) const
		{
			PairListT lst;

			for (auto& pr : m_data)
			{
				if (cb(pr.first, pr.second))
				{
					lst.addPair(pr.first, pr.second);
				}
			}

			return lst;
		}

		template <typename CB, typename = checkCBBool_t<CB, __FirstConstRef> >
		PairListT filterFirst(const CB& cb) const
		{
			PairListT lst;

			for (auto& pr : m_data)
			{
				if (cb(pr.first))
				{
					lst.addPair(pr.first, pr.second);
				}
			}

			return lst;
		}

		template <typename CB, typename = checkCBBool_t<CB, __SecondConstRef> >
		PairListT filterSecond(const CB& cb) const
		{
			PairListT lst;

			for (auto& pr : m_data)
			{
				if (cb(pr.second))
				{
					lst.addPair(pr.first, pr.second);
				}
			}

			return lst;
		}

		template <typename CB, typename = checkCBBool_t<CB, __FirstConstRef> >
		bool everyFirst(const CB& cb) const
		{
			for (auto& pr : m_data)
			{
				if (!cb(pr.first))
				{
					return false;
				}
			}

			return true;
		}

		template <typename CB, typename = checkCBBool_t<CB, __SecondConstRef> >
		bool everySecond(const CB& cb) const
		{
			for (auto& pr : m_data)
			{
				if (!cb(pr.second))
				{
					return false;
				}
			}

			return true;
		}

		template <typename CB, typename = checkCBBool_t<CB, __FirstConstRef> >
		bool anyFirst(const CB& cb) const
		{
			for (auto& pr : m_data)
			{
				if (cb(pr.first))
				{
					return true;
				}
			}

			return false;
		}

		template <typename CB, typename = checkCBBool_t<CB, __SecondConstRef> >
		bool anySecond(const CB& cb) const
		{
			for (auto& pr : m_data)
			{
				if (cb(pr.second))
				{
					return true;
				}
			}

			return false;
		}

		PairListT& sort(__CB_Sort_T<__PairType> cb)
		{
			__Super::sort(cb);

			return *this;
		}

		PairListT& sortFirst()
		{
			tagTrySort<__FirstType> trySort;
            __Super::sort([&](__PairRef pr1, __PairRef pr2) {
                return trySort(pr1.first, pr2.first);
			});

			return *this;
		}

		PairListT& sortFirst(__CB_Sort_T<__FirstType> cb)
		{
            __Super::sort([&](__PairRef pr1, __PairRef pr2) {
                return cb(pr1.first, pr2.first);
			});

			return *this;
		}

		PairListT& sortSecond()
		{
			tagTrySort<__SecondType> trySort;
            __Super::sort([&](__PairRef pr1, __PairRef pr2) {
                return trySort(pr1.second, pr2.second);
			});

			return *this;
		}

		PairListT& sortSecond(__CB_Sort_T<__SecondType> cb)
		{
            __Super::sort([&](__PairRef pr1, __PairRef pr2) {
                return cb(pr1.second, pr2.second);
			});

			return *this;
		}

	private:
		virtual void _toString(stringstream& ss, __PairConstRef pr) const override
		{
			tagSSTryLMove(ss) << '<' << pr.first << ", " << pr.second << '>';
		}

	private:
		template <class T = __ContainerType>
		class CAdaptor
		{
		public:
			CAdaptor(T& data, const PtrArray<__PairType>& ptrArray)
				: m_data(data)
				, m_ptrArray(ptrArray)
			{
			}

		private:
			T& m_data;

			const PtrArray<__PairType>& m_ptrArray;

			using __PairRef = decltype(*declval<T&>().begin())&;
			using __FirstRef = decltype(declval<T&>().begin()->first)&;
			using __SecondRef = decltype(declval<T&>().begin()->second)&;

		public:
			template <typename CB, typename = checkCBBool_t<CB, __FirstConstRef, __SecondConstRef> >
			SArray<__FirstType> firsts(const CB& cb) const
			{
				SArray<__FirstType> arr;
				for (auto& pr : m_data)
				{
					if (cb(pr.first, pr.second))
					{
						arr.add(pr.first);
					}
				}

				return arr;
			}

			template <typename CB, typename = checkCBBool_t<CB, __FirstConstRef>, typename = void>
			SArray<__FirstType> firsts(const CB& cb) const
			{
				return firsts([&](__FirstConstRef first, __SecondConstRef) {
					return cb(first);
				});
			}

			template <typename CB, typename = checkCBBool_t<CB, __FirstConstRef, __SecondConstRef> >
			SArray<__SecondType> seconds(const CB& cb) const
			{
				SArray<__SecondType> arr;
				for (auto& pr : m_data)
				{
					if (cb(pr.first, pr.second))
					{
						arr.add(pr.second);
					}
				}

				return arr;
			}

			template <typename CB, typename = checkCBBool_t<CB, __SecondConstRef>, typename = void>
			SArray<__SecondType> seconds(const CB& cb) const
			{
				return seconds([&](__FirstConstRef, __SecondConstRef second) {
					return cb(second);
				});
			}

			template <typename CB, typename = checkCBBool_t<CB, __PairRef>>
			void forEach(const CB& cb, int startPos = 0, int endPos = -1) const
			{
				m_ptrArray(startPos, endPos, cb);
			}

			template <typename CB, typename = checkCBVoid_t<CB, __PairRef>, typename = void>
			void forEach(const CB& cb, int startPos = 0, int endPos = -1) const
			{
				m_ptrArray(startPos, endPos, cb);
			}

			template <typename CB, typename = checkCBBool_t<CB, __FirstRef, __SecondRef>
				, typename = void, typename = void>
			void forEach(const CB& cb, int startPos = 0, int endPos = -1) const
			{
				m_ptrArray(startPos, endPos, [&](__PairRef pr) {
					return cb(pr.first, pr.second);
				});
			}

			template <typename CB, typename = checkCBVoid_t<CB, __FirstRef, __SecondRef>
				, typename = void, typename = void, typename = void>
			void forEach(const CB& cb, int startPos = 0, int endPos = -1) const
			{
				m_ptrArray(startPos, endPos, [&](__PairRef pr) {
					cb(pr.first, pr.second);

					return true;
				});
			}

			template <typename CB, typename = checkCBBool_t<CB, __FirstRef>>
			void forFirst(const CB& cb, int startPos = 0, int endPos = 0) const
			{
				m_ptrArray([&](__PairRef pr) {
					return cb(pr.first);
				});
			}

			template <typename CB, typename = checkCBVoid_t<CB, __FirstRef>, typename = void>
			void forFirst(const CB& cb, int startPos = 0, int endPos = 0) const
			{
				m_ptrArray([&](__PairRef pr) {
					cb(pr.first);

					return true;
				});
			}

			template <typename CB, typename = checkCBBool_t<CB, __SecondRef>>
			void forSecond(const CB& cb, int startPos = 0, int endPos = 0) const
			{
				m_ptrArray([&](__PairRef pr) {
					return cb(pr.second);
				});
			}

			template <typename CB, typename = checkCBVoid_t<CB, __SecondRef>, typename = void>
			void forSecond(const CB& cb, int startPos = 0, int endPos = 0) const
			{
				m_ptrArray([&](__PairRef pr) {
					cb(pr.second);

					return true;
				});
			}
		};

		CAdaptor<> m_adaptor = CAdaptor<>(m_data, __Super::m_ptrArray);
		inline CAdaptor<>& adaptor()
		{
			return m_adaptor;
		}
		inline CAdaptor<const __ContainerType>& adaptor() const
		{
			return (CAdaptor<const __ContainerType>&)m_adaptor;
		}
	};
}

#endif // __PairList_H
