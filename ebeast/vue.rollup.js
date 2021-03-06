// This Source Code Form is licensed MPL-2.0: http://mozilla.org/MPL/2.0

import vue from 'rollup-plugin-vue';		   // compile SFC.vue files
import resolve from '@rollup/plugin-node-resolve'; // find vue-runtime-helpers/dist/normalize-component
import scss from 'rollup-plugin-scss';		   // compile scss to css

export default {
  // https://rollupjs.org/guide/en/#big-list-of-options
  output: {
    sourcemap: 'inline',
    format: 'esm',
  },
  plugins: [
    vue ({
      // https://rollup-plugin-vue.vuejs.org/options.html
      css: false,
      needMap: false,
      // normalizeComponent needs ./node_modules/vue-runtime-helpers/dist/index.mjs import
      normalizer: "Vue.__vue_normalize__", // see index.html for vue-runtime-helpers.mjs
      // use node-sass, dart-sass has no support for { functions:... }
      style: { preprocessOptions: {
	scss: {
	  sass: require ('node-sass'),           // supports custom functions
	  // https://github.com/sass/node-sass#options
	  sourceMapEmbed: true,
	  outputStyle: 'nested',                 // nested, expanded, compact, compressed
	  sourceComments: false,
	  functions: require ("chromatic-sass"), // provide node-sass color functions
	},
      }, },
      // data: { css: '/* globals... */', }, // increases line numbers in source maps
    }),
    scss (),
    resolve(),
  ],
  // indicate which modules should be treated as external
  external: id => {
    // treat all regular file imports as external
    return id.search (/\?/) < 0;
  },
};

const external = [
  '/jsbse.js',
  '/vue.mjs',
  '/markdown-it.mjs',
  '/util.js',
];
external;
