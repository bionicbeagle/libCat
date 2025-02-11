#include <cat/linux>

auto nix::block_all_signals() -> signals_mask_set {
    signals_mask_set current_mask;
    // Most Linux runtimes use an `app_mask` here which excludes signals 32-34,
    // which are used for something by pthreads. Because libCat doesn't use
    // pthreads, it simply blocks all signals here, for now.
    sys_rt_sigprocmask(signal_action::block, &all_signals_mask, &current_mask);
    return current_mask;
}
