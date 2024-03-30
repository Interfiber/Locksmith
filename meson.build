project(
    'locksmith',
    'cpp',
    version : '1.0.0',
    default_options : [
        'buildtype=debug',
        'optimization=0',
        'cpp_std=c++17'
    ],
    meson_version: '>= 0.60.0',
)

cmake = import('cmake')


funchook = cmake.subproject('funchook')

funchook_lib = funchook.dependency('funchook-shared')

executable('locksmith',
           'main.cpp'
)

library('locksmith_inject', 'locksmith_inject.cpp', dependencies: [ funchook_lib ])

