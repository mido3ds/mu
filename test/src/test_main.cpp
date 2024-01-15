#include <mu/utils.h>

int main() {
    mu_test_suite("main tests");

    mu_defer(mu::log_debug("runs at end"));

    if (1 == 0) {
        mu_unreachable();
    }

    mu_test(1+1 == 2);

    mu::Arr<int, 4> arr{1,1,2,2};
    mu::log_debug("arr={}", arr);

    mu::Vec<int> vec{1,1,2,2};
    mu::log_debug("vec={}", vec);

    mu::Set<int> set{1,1,2,2};
    mu::log_debug("set={}", set);

    mu::Map<int, int> map{{1,1},{2,2}};
    mu::log_debug("map={}", map);

    mu::log_debug("{}", mu::dir_list_files(mu::folder_config(mu::memory::tmp()), mu::memory::tmp()));

    mu::Str str1 = "HELLO";
    auto str2 = str1;
    mu::str_to_lower(str2);
    mu_test_msg(str2 == "hello", "can lower str");

    auto timer = mu::timer_new();
    mu::thread_sleep_millis(100);
    mu::log_debug("should have slept for 100ms, actually slept for {}ms", mu::timer_elapsed(timer));
}
