#ifndef HX_CFFI_LOADER_H
#define HX_CFFI_LOADER_H


typedef void *(*ResolveProc)(const char *inName);
static ResolveProc sResolveProc = 0;

#ifdef NEKO_WINDOWS

#include <windows.h>
 
void *LoadFunc(const char *inName)
{
   static char *modules[] = { 0, "hxcpp", "hxcpp-debug" };
   for(int i=0; i<3 && sResolveProc==0; i++)
   {
      HMODULE handle = GetModuleHandle(modules[i]);
      if (handle)
      {
         sResolveProc = (ResolveProc)GetProcAddress(handle,"hx_cffi");
         if (sResolveProc==0)
            FreeLibrary(handle);
      }
   }
   if (sResolveProc==0)
   {
      HMODULE handle = LoadLibrary("CFFI.dll");
      if (handle)
      {
         sResolveProc = (ResolveProc)GetProcAddress(handle,"hx_cffi");
         if (sResolveProc==0)
            FreeLibrary(handle);
      }
   }
   if (sResolveProc==0)
   {
      fprintf(stderr,"Could not link plugin to process (%s)",__FILE__);
      exit(1);
   }
   return sResolveProc(inName);
}

#else // not windows

#ifdef NEKO_LINUX
#define EXT "dso"
#else
#define EXT "dylib"
#endif

#include <dlfcn.h>

void *LoadFunc(const char *inName)
{
   if (sResolveProc==0)
   {
      sResolveProc = (ResolveProc)dlsym(RTLD_DEFAULT,"hx_cffi");
   }
   if (sResolveProc==0)
   {
      void *handle = dlopen("hxcpp." EXT,RTLD_NOW);
      if (handle)
      {
         sResolveProc = (ResolveProc)dlsym(handle,"hx_cffi");
printf("PROC : %p\n",sResolveProc);
      }
   }
   if (sResolveProc==0)
   {
      void *handle = dlopen("CFFI." EXT ,RTLD_NOW);
      if (handle)
         sResolveProc = (ResolveProc)dlsym(handle,"hx_cffi");
   }
   if (sResolveProc==0)
   {
      fprintf(stderr,"Could not link plugin to process (%s)",__FILE__);
      exit(1);
   }
   return sResolveProc(inName);
}

#undef EXT

#endif

 
#define DEFFUNC(name,ret,def_args,call_args) \
typedef ret (*FUNC_##name)def_args; \
extern FUNC_##name name; \
ret IMPL_##name def_args \
{ \
   name = (FUNC_##name)LoadFunc(#name); \
   return name call_args; \
}\
FUNC_##name name = IMPL_##name;
 



#endif
