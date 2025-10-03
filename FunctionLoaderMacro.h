#pragma once

#ifdef HOT_RELOAD

// Macro to define function (use in header file)
#define DEF_RELOADABLE_FUNCTION(FuncReturn, FuncName, ...)							\
																					\
/* When HOT_RELOAD is defined, this template function is implemented */				\
/* in place of the actual function declaration */									\
template<typename... Args>															\
FuncReturn FuncName(Args&&... args)													\
{																					\
	/* Define the function pointer type */											\
	using FuncType = FuncReturn(*)(Args...);										\
	/* Initialize static object that interfaces with the DLL */						\
	static FunctionLoader functionLoader("_" #FuncName "DLL", #FuncName ".dll");	\
	/* Verify if there is an updated DLL to be loaded and load it if necessary */	\
	functionLoader.CheckNewDLL();													\
	/* Get the function pointer */													\
	FuncType funcPtr = (FuncType)functionLoader.GetPtr();							\
	/* Verify if the pointer points to a valid function */							\
	if (funcPtr)																	\
		/* If it does, call the function */											\
		return funcPtr(args...);													\
	else																			\
		/* If it doesn't, return an empty return type */							\
		return FuncReturn {};														\
}																					\
																					\
/* Declare the actual function with a modified name, to be exported */				\
extern "C" __declspec(dllexport) FuncReturn _##FuncName##DLL(__VA_ARGS__)

// Macro to implement function (use in source file)
// Create the signature of the exported function, with a modified name
#define IMPL_RELOADABLE_FUNCTION(FuncReturn, FuncName, ...) FuncReturn _##FuncName##DLL(__VA_ARGS__)

#else

// Just define the function signatures as usual
#define DEF_RELOADABLE_FUNCTION(FuncReturn, FuncName, ...) FuncReturn FuncName(__VA_ARGS__)
#define IMPL_RELOADABLE_FUNCTION(FuncReturn, FuncName, ...) FuncReturn FuncName(__VA_ARGS__)

#endif

