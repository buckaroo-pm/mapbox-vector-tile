load('//:buckaroo_macros.bzl', 'buckaroo_deps')

prebuilt_cxx_library(
  name = 'vector-tile', 
  header_namespace = 'mapbox', 
  header_only = True, 
  exported_headers = subdir_glob([
    ('include/mapbox', '**/*.hpp'), 
  ]), 
  licenses = [
    'LICENSE', 
  ], 
  deps = buckaroo_deps(), 
  visibility = [
    'PUBLIC', 
  ], 
)
