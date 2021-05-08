#pragma once

#define BIND(a_method, ...) a_vm.BindNativeMethod(obj, #a_method##sv, a_method __VA_OPT__(, ) __VA_ARGS__)
