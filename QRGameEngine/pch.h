#pragma once
//Basic
#include <string>
#include <vector>
#include <string>
#include <iostream>
#include <assert.h>
//#include <unordered_map>
//#include <map>
//#include <unordered_set>
#include <fstream>
#include <thread>
#include <mutex>
#include <functional>
#include <stack>
#include <chrono>
#include <concepts>
#include <algorithm>
#include <set>
#include <array>
#include <list>
#include <ranges>
#include <tuple>
#include <algorithm>

#include "Vendor/Include/ankerl/unordered_dense.h"
namespace qr {
	template <typename Key, typename T, class Hash = ankerl::unordered_dense::hash<Key>,
		class KeyEqual = std::equal_to<Key>,
		class AllocatorOrContainer = std::allocator<std::pair<Key, T>>,
		class Bucket = ankerl::unordered_dense::bucket_type::standard,
		class BucketContainer = ankerl::unordered_dense::detail::default_container_t>
	using unordered_map = ankerl::unordered_dense::map<Key, T, Hash, KeyEqual, AllocatorOrContainer, Bucket, BucketContainer>;

	template <class Key, class Hash = ankerl::unordered_dense::hash<Key>, 
		class KeyEqual = std::equal_to<Key>, 
		class AllocatorOrContainer = std::allocator<Key>, 
		class Bucket = ankerl::unordered_dense::bucket_type::standard, 
		class BucketContainer = ankerl::unordered_dense::detail::default_container_t>
	using unordered_set = ankerl::unordered_dense::set<Key, Hash, KeyEqual, AllocatorOrContainer, Bucket, BucketContainer>;
}

//Windows
#include <Windows.h>
#include <wrl/client.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>