project('lisiblepng', 'c', default_options: ['c_std=c18', 'warning_level=3', 'default_library=static'], version:  '0.2.0')

subdir('lisiblepng')

if not meson.is_cross_build()
subdir('lisiblepng-bin')
endif
