<!-- This Source Code Form is licensed MPL-2.0: http://mozilla.org/MPL/2.0 -->

<docs>
  # B-MENUITEM
  A menuitem element to be used as a descendant of a [B-CONTEXTMENU](#b-contextmenu).
  The menuitem can be activated via keyboard focus or mouse click and will notify
  its B-CONTEXTMENU about the click and its `uri`, unless the `@click.prevent`
  modifier is used.
  If no `uri` is specified, the B-CONTEXTMENU will still be notified to be closed,
  unless `$event.preventDefault()` is called.
  ## Props:
  *uri*
  : Descriptor for this menuitem that is passed on to its B-CONTEXTMENU `onclick`.
  *kbd*
  : Hotkey to display next to the menu item and to use for the context menu kbd map.
  *disabled*
  : Boolean flag indicating disabled state.
  *fa*, *mi*, *bc*, *uc*
  : Shorthands icon properties that are forwarded to a [B-ICON](#b-icon) used inside the menuitem.
  ## Events:
  *click*
  : Event emitted on keyboard/mouse activation, use `preventDefault()` to avoid closing the menu on clicks.
  ## Slots:
  *default*
  : All contents passed into this slot will be rendered as contents of this element.
</docs>

<style lang="scss">
  @import 'mixins.scss';
  body button.b-menuitem { //* since menus are often embedded, this needs high specificity */
    display: inline-flex; flex: 0 0 auto; flex-wrap: nowrap;
    margin: 0; padding: $b-menu-vpad $b-menu-hpad; text-align: left;
    //* button-reset */
    background: transparent; cursor: pointer; user-select: none; outline: none;
    border: 1px solid transparent;
    color: $b-menu-foreground;
    kbd { color: contrast-darken($b-menu-foreground, 1.5); }
    &:not([disabled]) {
      .b-icon { color: $b-menu-fill; }
      &:focus {
	background-color: $b-menu-focus-bg; color: $b-menu-focus-fg; outline: none;
	kbd { color: inherit; }
	border: 1px solid darken($b-menu-focus-bg, 50%);
	.b-icon { color: $b-menu-focus-fg; }
      }
      &.active, &:active, &:focus.active, &:focus:active {
	background-color: $b-menu-active-bg; color: $b-menu-active-fg; outline: none;
	kbd { color: inherit; }
	border: 1px solid darken($b-menu-active-bg, 50%);
	.b-icon { color: $b-menu-active-fg; }
      }
    }
    &[disabled], &[disabled] * {
      color: $b-menu-disabled;
      .b-icon { color: $b-menu-disabled-fill; }
    }
    flex-direction: row; align-items: baseline;
    & > .b-icon:first-child { margin: 0 $b-menu-spacing 0 0; }
    .kbdspan {
      flex-grow: 1;
      text-align: right;
      padding-left: 2.5em;
      kbd {
	font-family: inherit;
	font-weight: 400;
      }
    }
  }
  body .b-menurow-turn button.b-menuitem {
    flex-direction: column; align-items: center;
    & > .b-icon:first-child { margin: 0 0 $b-menu-spacing 0; }
  }
  body .b-menurow-noturn button.b-menuitem {
    .menulabel { min-width: 2em; } //* this aligns blocks of 2-digit numbers */
    & > .b-icon:first-child { margin: 0 $b-menu-tightspace 0 0; }
  }
</style>

<template>
  <button class="b-menuitem"
	  :disabled="isdisabled()"
	  @mouseenter="focus"
	  @keydown="Util.keydown_move_focus"
	  @click="onclick" >
    <b-icon :class='iconclass' :ic="ic" :fa="fa" :mi="mi" :bc="bc" :uc="uc" v-if="menudata.showicons" />
    <span class="menulabel"><slot /></span>
    <span v-if="kbd !== undefined" class="kbdspan"><kbd v-if="kbd">{{ Util.display_keyname (kbd) }}</kbd></span>
  </button>
</template>

<script>
const STR = { type: String, default: '' }; // empty string default
export default {
  name: 'b-menuitem',
  props: { 'uri': {}, 'disabled': { type: Boolean },
	   iconclass: STR, ic: STR, fa: STR, mi: STR, bc: STR, uc: STR,
	   kbd: { type: String }, },
  inject: { menudata: { from: 'b-contextmenu.menudata',
			default: { showicons: true, keepmounted: false, checkeduris: {},
				   isdisabled: () => false, onclick: undefined, }, },
  },
  methods: {
    focus() {
      if (this.$el && !this.isdisabled())
	this.$el.focus();
    },
    onclick (event) 	{ return this.menudata.onclick?.call (this, event); },
    isdisabled ()	{ return this.menudata.isdisabled.call (this); },
    kbd_hotkey() {
      return this.kbd;
    },
    /// Extract menu item label (without icon/kbd infos).
    get_text() {
      const filter = e => {
	if (e.nodeName == 'KBD')
	  return false;
	if (e.getAttribute ('role') == 'icon')
	  return false;
      };
      return Util.element_text (this.$el, filter);
    },
  },
};
</script>
