project(
  'voluminous',
  'cpp',
  version : '0.1.0',
  default_options : [
    'cpp_std=c++17',
    'cpp_args=-fno-exceptions -pedantic'
  ]
)

incdir = include_directories('include')

sources = [
  'src/device.cpp',
  'src/pulse_api.cpp'
]

libpulse = dependency('libpulse')

dependencies = [
  libpulse
]

executable(
  'voluminous',
  ['src/voluminous.cpp', sources],
  include_directories : incdir,
  dependencies : dependencies
)
