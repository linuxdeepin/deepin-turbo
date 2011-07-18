#define DECL_EXPORT extern "C" __attribute__ ((__visibility__("default")))
extern "C"
{
    DECL_EXPORT bool lock(const char * binaryName)
    {
        return true;
    }

    DECL_EXPORT void unlock()
    {
    }
}
