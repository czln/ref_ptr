# ref_ptr

a simple smart pointer that can handle array elements,
which was a feature in c++17 (yet stil cause memory problem until c++20).

support copy and move assignment, each copy would add the inner
counter. when the counter is 0, it releases the source automatically.
if it's a single object, it calls `delete`. if it's an array of objects,
it calls `delete []`