* Use Allman style
* Don't use any non-standard-library libraries
* Keep everything in the args namespace
* Keep it simple and fast
* Your changes may be licensed how you wish as long as the project itself can
  remain MIT-licensed (essentially meaning your license for your changes must be
  either compatible with the MIT license or you must dual-license and the code
  put in here must be compatible with the MIT)
* Changes must remain compatible with C++11.
* Do not make defensive changes for conditions that will never happen. Examples:
  * Don't check for null pointers for things that can not be null, like argv[i]
    where i is in the range [0, argc). If the C standard guarantees a shape for
    argv and arc, we don't need to additionally check those ourselves.
  * Don't check for size_t overflow on string arguments. This will never happen.
* For behavior bug fixes, you must add code that will fail on master but succeed
  with the fix. Do **not** just test the newly added code.
* For difficult-to-test code, such as undefined behavior fixes or code that
  leads to corruption that isn't immediately visible, you **must** include in
  the description of the PR (or an issue that precedes the PR) a reproduction
  that shows exactly how to trigger the problem condition.
* Don't add overly-defensive code. The problem condition must have the ability
  to be practically triggered before we want to worry about it. If the bug would
  take terabytes of RAM or the heat death of the universe to hit, don't bother
  fixing it.
* Avoid unnecessary indirection and abstraction.
* Try to avoid growing the code. It's already too big as it is. Unless there's a
  very good reason (argued for in the PR), PRs should not add a ton of non-test
  code.
