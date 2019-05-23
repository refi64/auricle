musivid_sources = [
  'main.c',
  'musivid-image-section.c',
  'musivid-music-file.c',
  'musivid-music-row.c',
  'musivid-music-table.c',
  'musivid-notification.c',
  'musivid-options-editor.c',
  'musivid-progress-row.c',
  'musivid-progress-view.c',
  'musivid-renderer.c',
  'musivid-render-options.c',
  'musivid-utils.c',
  'musivid-window.c',
]

musivid_deps = [
  dependency('gio-2.0', version: '>= 2.50'),
  dependency('gtk+-3.0', version: '>= 3.22'),
  dependency('gstreamer-1.0'),
  dependency('gstreamer-app-1.0'),
  dependency('gstreamer-video-1.0'),
]

gnome = import('gnome')

musivid_sources += gnome.compile_resources('musivid-resources',
  'musivid.gresource.xml',
  c_name: 'musivid'
)

executable('musivid', musivid_sources,
  dependencies: musivid_deps,
  install: true,
)
