#pragma once

#ifdef COMPILER_APPLE_CLANG
#include <experimental/memory_resource>
namespace std { namespace pmr = experimental::pmr; }
#else
#include <memory_resource>
#endif

#include <vector>
#include <array>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <fstream>
#include <functional>
#include <filesystem>

#include <fmt/format.h>
#include <fmt/ostream.h>

#define mu_count_of(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#define __CONCAT_MACRO_2(x, y) x##y
#define __CONCAT_MACRO_(x, y) __CONCAT_MACRO_2(x, y)

// defers the given code/block of code to the end of the current scopre
#define mu_defer(code)   const mu::Deferrer_ __CONCAT_MACRO_(_defer_, __LINE__) ([&]{code;})

#if COMPILER_MSVC
	#define mu_debugger_breakpoint() __debugbreak()
#elif COMPILER_CLANG || COMPILER_GNU || COMPILER_APPLE_CLANG
	#define mu_debugger_breakpoint() __builtin_trap()
#else
	#error unknown compiler
#endif

#ifdef NDEBUG
	#define mu_assert_msg(expr, message) ((void)0)
	#define mu_assert(expr) ((void)0)
#else
	#define mu_assert_msg(expr, message) { if (expr) {} else { mu::panic("[ASSERT] {}:{} ({}) {}", __FILE__, __LINE__, #expr, message); mu_debugger_breakpoint(); } }
	#define mu_assert(expr) { if (expr) {} else { mu::panic("[ASSERT] {}:{} ({})",__FILE__, __LINE__, #expr); mu_debugger_breakpoint(); } }
#endif

#define mu_unreachable() mu_assert_msg(false, "unreachable")

#define mu_test_suite(name) mu_defer(fmt::print(stderr, "====> \"" name "\" all tests passed\n"))
#define mu_test(expr) { if (expr) { fmt::print(stderr, "[OK] ({})\n", #expr); } else { mu::panic("[FAIL] {}:{} ({})",__FILE__, __LINE__, #expr); mu_debugger_breakpoint(); } }
#define mu_test_msg(expr, message) { if (expr) { fmt::print(stderr, "[OK] ({}) {}\n", #expr, message); } else { mu::panic("[FAIL] {}:{} ({}) {}",__FILE__, __LINE__, #expr, message); mu_debugger_breakpoint(); }  }

namespace mu {
	template<typename T>
	using Box = std::unique_ptr<T>;

	template <class T, class... Types, std::enable_if_t<!std::is_array_v<T>, int> = 0>
	[[nodiscard]] Box<T>
	box_new(Types&&... _Args) {
		return Box<T>(new T(std::forward<Types>(_Args)...));
	}

	template<typename T, size_t N>
	using Arr = std::array<T, N>;

	using Str = std::basic_string<char, std::char_traits<char>, std::pmr::polymorphic_allocator<char>>;
	using StrView = std::string_view;

	template<typename T>
	using Vec = std::vector<T, std::pmr::polymorphic_allocator<T>>;

	template<typename K, typename V>
	using Map = std::unordered_map<
		K, V,
		std::hash<K>,
		std::equal_to<K>,
		std::pmr::polymorphic_allocator< std::pair<const K, V> >
	>;

	template<typename K>
	using Set = std::unordered_set<
		K,
		std::hash<K>,
		std::equal_to<K>,
		std::pmr::polymorphic_allocator<K>
	>;

	/////////////////////////////////////////////////////////////////////////////////

	namespace memory {
		using Allocator = std::pmr::memory_resource;

		class Arena : public Allocator {
			static constexpr size_t BLOCK_SIZE = 4096;
		public:
			virtual ~Arena() noexcept override;
		protected:
			virtual void* do_allocate(std::size_t size, std::size_t alignment) override;
			virtual void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override { }
			virtual bool do_is_equal(const Allocator& other) const noexcept override {
				return this == &other;
			}

			struct Node {
				void* mem_ptr;
				size_t mem_size;
				uint8_t* alloc_head;
				Node* next;
			};

			Node* head = nullptr;
		};

		class LibcAllocator : public Allocator {
			virtual void* do_allocate(std::size_t bytes, std::size_t _alignment) override {
				return ::malloc(bytes);
			}

			virtual void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override {
				::free(p);
			}

			virtual bool do_is_equal(const Allocator& other) const noexcept override {
				return this == &other;
			}
		};

		Allocator* tmp();

		// make sure any container that uses tmp has been destructed before resetting
		void reset_tmp();

		inline static Allocator*
		default_allocator() {
			return std::pmr::get_default_resource();
		}

		inline static Allocator*
		set_default_allocator(Allocator* new_allocator) {
			const auto x = std::pmr::get_default_resource();
			std::pmr::set_default_resource(new_allocator);
			return x;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////

	template<class T>
	inline static void
	vec_remove_unordered(Vec<T>& v, int i) {
		auto last = v.rbegin();
		if (i != v.size()-1) {
			v[i] = std::move(*last);
		}
		v.pop_back();
	}

	inline static void
	str_replace(Str& self, StrView search, StrView replace) {
		size_t pos = 0;
		while ((pos = self.find(search, pos)) != Str::npos) {
			self.replace(pos, search.length(), replace);
			pos += replace.length();
		}
	}

	inline static void
	str_replace(Str& self, char search, char replace) {
		size_t pos = 0;
		while ((pos = self.find(search, pos)) != Str::npos) {
			self[pos] = replace;
			pos++;
		}
	}

	// appends the formatted string to the end of self
	template<typename ... Args>
	inline static void
	str_push(Str& self, StrView format_str, const Args& ... args) {
		fmt::vformat_to(std::back_inserter(self), format_str, fmt::make_format_args(args...));
	}

	template<typename ... Args>
	inline static Str
	str_format(memory::Allocator* allocator, StrView format_str, const Args& ... args) {
		Str self(allocator);
		str_push(self, format_str, args...);
		return self;
	}

	template<typename ... Args>
	inline static Str
	str_format(StrView format_str, const Args& ... args) {
		return str_format(memory::default_allocator(), format_str, args...);
	}

	template<typename ... Args>
	inline static Str
	str_tmpf(StrView format_str, const Args& ... args) {
		return str_format(memory::tmp(), format_str, args...);
	}

	// trim spaces from start (in place)
	inline static void
	str_ltrim_spaces(Str& s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
			return !std::isspace(ch);
		}));
	}

	// trim spaces from end (in place)
	inline static void
	str_rtrim_spaces(Str& s) {
		s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
			return !std::isspace(ch);
		}).base(), s.end());
	}

	// trim spaces from both ends (in place)
	inline static void
	str_trim_spaces(Str& s) {
		str_rtrim_spaces(s);
		str_ltrim_spaces(s);
	}

	inline static void
	str_to_lower(Str& s) {
		std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
	}

	template<typename T>
	inline static bool
	operator==(const Vec<T>& aa, const Vec<T>& bb) {
		if (aa.size() != bb.size()) {
			return false;
		}
		for (int i = 0; i < aa.size(); i++) {
			if (aa[i] != bb[i]) {
				return false;
			}
		}
		return true;
	}

	template<typename T>
	inline static bool
	operator!=(const Vec<T>& aa, const Vec<T>& bb) {
		return (aa == bb) == false;
	}

	/////////////////////////////////////////////////////////////////////////////////

	// captures the top frames_count callstack frames and writes it to the given frames pointer
	size_t _callstack_capture(void** frames, size_t frames_count);
	void _callstack_print_to(void** frames, size_t frames_count);

	// prints the given message and the call stack then terminates the program
	template<typename ... TArgs>
	[[noreturn]] inline static void
	panic(StrView fmt, TArgs&& ... args) {
		const auto str = str_tmpf(fmt, args...);

		constexpr int frames_count = 20;
		void* frames[frames_count];
		_callstack_capture(frames, frames_count);

		fmt::print(stderr, "[PANIC]: {}\n", str);
		_callstack_print_to(frames, frames_count);

		mu_debugger_breakpoint();
		abort();
	}

	/////////////////////////////////////////////////////////////////////////////////

	struct ILogger {
		virtual void log_debug(StrView str)   = 0;
		virtual void log_info(StrView str)    = 0;
		virtual void log_warning(StrView str) = 0;
		virtual void log_error(StrView str)   = 0;
	};

	extern ILogger* log_global_logger;

	template<typename... TArgs>
	inline static void
	log_debug([[maybe_unused]] StrView fmt, [[maybe_unused]] TArgs&&... args) {
		#ifdef DEBUG
		if (log_global_logger) {
			log_global_logger->log_debug(str_tmpf(fmt, args...));
		} else {
			fmt::print(stderr, "[debug] {}\n", str_tmpf(fmt, args...));
		}
		#endif
	}

	template<typename... TArgs>
	inline static void
	log_info(StrView fmt, TArgs&&... args) {
		if (log_global_logger) {
			log_global_logger->log_info(str_tmpf(fmt, args...));
		} else {
			fmt::print("[info] {}\n", str_tmpf(fmt, args...));
		}
	}

	template<typename... TArgs>
	inline static void
	log_warning(StrView fmt, TArgs&&... args) {
		if (log_global_logger) {
			log_global_logger->log_warning(str_tmpf(fmt, args...));
		} else {
			fmt::print(stderr, "[warning] {}\n", str_tmpf(fmt, args...));
		}
	}

	template<typename... TArgs>
	inline static void
	log_error(StrView fmt, TArgs&&... args) {
		if (log_global_logger) {
			log_global_logger->log_error(str_tmpf(fmt, args...));
		} else {
			fmt::print(stderr, "[error] {}\n", str_tmpf(fmt, args...));
		}
	}

	/////////////////////////////////////////////////////////////////////////////////

	template <typename F>
	struct Deferrer_ {
		F f;
		inline Deferrer_(F f): f(f) {}
		inline ~Deferrer_() { f(); }
	};

	Str folder_config(memory::Allocator* allocator = memory::default_allocator());

	inline static void
	path_normalize(Str& path) {
		str_replace(path, '\\', '/');
	}

	inline static StrView
	file_get_base_name(StrView path) {
		int i = path.size()-1;
		while (i >= 0) {
			if (path[i] == '/' || path[i] == '\\') {
				break;
			}
			i--;
		}
		return path.substr(i+1);
	}

	inline static Str
	file_content_str(const char* path, memory::Allocator* allocator = memory::default_allocator()) {
		auto file = fopen(path, "rb");
		if (file == nullptr) {
			panic("failed to open file '{}' for reading", path);
		}
		mu_defer(fclose(file));

		if (fseek(file, 0, SEEK_END)) {
			panic("failed to seek in file '{}' for reading", path);
		}
		const auto expected_size = ftell(file);
		if (fseek(file, 0, SEEK_SET)) {
			panic("failed to seek back in file '{}' for reading", path);
		}

		Str content(expected_size, 0, allocator);
		const auto read_size = fread(content.data(), 1, expected_size, file);
		if (read_size != expected_size) {
			panic("failed to read file '{}', expected to read {} bytes but got {} bytes", path, expected_size, read_size);
		}

		return content;
	}

	inline static Vec<Str>
	dir_list_files_with(
		StrView dir_abs_path,
		std::function<bool(StrView)> predicate,
		memory::Allocator* allocator = memory::default_allocator()
	) {
		Vec<Str> out;

		for (const auto & entry : std::filesystem::directory_iterator(dir_abs_path)) {
			if (entry.is_regular_file()) {
				auto filename = entry.path().filename().string();
				auto path = entry.path().string();

				if (predicate(filename)) {
					out.push_back(Str(path, allocator));
				}
			}
		}

		return out;
	}

	inline static Vec<Str>
	dir_list_files(
		StrView dir_abs_path,
		memory::Allocator* allocator = memory::default_allocator()
	) {
		return dir_list_files_with(dir_abs_path, [](auto){ return true; }, allocator);
	}
}

namespace fmt {
	template<typename T>
	struct formatter<mu::Vec<T>> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const mu::Vec<T> &self, FormatContext &ctx) {
			fmt::format_to(ctx.out(), "Vec<{}>[{}]{{", typeid(T).name(), self.size());
			for (int i = 0; i < self.size(); i++) {
				fmt::format_to(ctx.out(), "[{}]={}{}", i, self[i], (i == self.size()-1? "":", "));
			}
			return fmt::format_to(ctx.out(), "}}");
		}
	};

	template<typename T, size_t N>
	struct formatter<mu::Arr<T, N>> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const mu::Arr<T, N> &self, FormatContext &ctx) {
			fmt::format_to(ctx.out(), "Arr<{}>[{}]{{", typeid(T).name(), self.size());
			for (int i = 0; i < self.size(); i++) {
				fmt::format_to(ctx.out(), "[{}]={}{}", i, self[i], (i == self.size()-1? "":", "));
			}
			return fmt::format_to(ctx.out(), "}}");
		}
	};

	template<typename K, typename V>
	struct formatter<mu::Map<K, V>> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const mu::Map<K, V> &self, FormatContext &ctx) {
			auto s = self.size();
			fmt::format_to(ctx.out(), "Map<{} -> {}>[{}]{{", typeid(K).name(), typeid(V).name(), s);
			for (const auto& [k, v] : self) {
				fmt::format_to(ctx.out(), "[{}]={}{}", k, v, (s == 1? "":", "));
				s--;
			}
			return fmt::format_to(ctx.out(), "}}");
		}
	};

	template<typename T>
	struct formatter<mu::Set<T>> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const mu::Set<T> &self, FormatContext &ctx) {
			const auto s = self.size(); int i = 0;
			fmt::format_to(ctx.out(), "Set<{}>[{}]{{", typeid(T).name(), s);
			for (const auto& v : self) {
				fmt::format_to(ctx.out(), "[{}]={}{}", i, v, (i == s-1? "":", "));
				i++;
			}
			return fmt::format_to(ctx.out(), "}}");
		}
	};
}
