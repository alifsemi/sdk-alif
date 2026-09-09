# Copyright (C) Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license
#

"""Generate src/ui_xml.c from ui/screen_main.xml.

The Alif LVGL fork ships no runtime XML loader (lv_xml_*), so the declarative UI
in screen_main.xml is exported to C offline and compiled. This generator maps a
broad, declarative subset of LVGL to the equivalent create/set calls.

Coverage
--------
* Any standard LVGL v9 widget listed in CREATE can be created, sized, aligned
  (align + x/y offsets), styled (the *entire* local style API, including
  part/state selectors), flagged, put into a state, and nested to any depth.
* Widgets with simple scalar properties (value, range, text, options, src, ...)
  have declarative setters - see SPECIAL.
* Composite containers with a bespoke child API are supported: <lv_tabview> via
  <tab> children, <lv_win> (children go into the content area), and <lv_list>
  via <list_button>/<list_text> children.
* Things that are inherently code - custom draw callbacks, events, animations,
  chart series / table cell / canvas pixel data - cannot be expressed
  declaratively; using an unknown tag or attribute is a hard error.

Style attributes
----------------
Any ``style_<prop>`` attribute maps to ``lv_obj_set_style_<prop>``; the value
kind is inferred from the property name (``*_color`` -> color, ``*_opa`` ->
opacity, coordinates support ``%``/``content``, known enums are mapped). A
part/state selector may be appended with ``__`` tokens, e.g.
``style_bg_color__indicator`` or ``style_bg_color__knob__pressed``.

Usage:
    python scripts/gen_ui_xml.py ui/screen_main.xml src/ui_xml.c
    python scripts/gen_ui_xml.py ui/screen_main.xml        # writes to stdout
"""

import re
import sys
import xml.etree.ElementTree as ET

TAB = "\t"

# Widget constructors. Any tag here can be created, sized, aligned, flagged,
# put in a state, styled and nested; value setters come from SPECIAL.
CREATE = {
    "lv_obj": "lv_obj_create",
    "lv_label": "lv_label_create",
    "lv_button": "lv_button_create",
    "lv_buttonmatrix": "lv_buttonmatrix_create",
    "lv_image": "lv_image_create",
    "lv_imagebutton": "lv_imagebutton_create",
    "lv_line": "lv_line_create",
    "lv_bar": "lv_bar_create",
    "lv_slider": "lv_slider_create",
    "lv_arc": "lv_arc_create",
    "lv_switch": "lv_switch_create",
    "lv_checkbox": "lv_checkbox_create",
    "lv_dropdown": "lv_dropdown_create",
    "lv_roller": "lv_roller_create",
    "lv_textarea": "lv_textarea_create",
    "lv_spinbox": "lv_spinbox_create",
    "lv_spinner": "lv_spinner_create",
    "lv_led": "lv_led_create",
    "lv_list": "lv_list_create",
    "lv_menu": "lv_menu_create",
    "lv_table": "lv_table_create",
    "lv_chart": "lv_chart_create",
    "lv_calendar": "lv_calendar_create",
    "lv_tabview": "lv_tabview_create",
    "lv_tileview": "lv_tileview_create",
    "lv_win": "lv_win_create",
    "lv_scale": "lv_scale_create",
    "lv_spangroup": "lv_spangroup_create",
    "lv_keyboard": "lv_keyboard_create",
}

# Attributes handled generically for every widget.
GENERIC_ATTRS = {
    "name", "width", "height", "align", "x", "y", "flags", "states",
    "checked", "disabled",
}

PALETTE = {
    "red", "pink", "purple", "deep_purple", "indigo", "blue", "light_blue",
    "cyan", "teal", "green", "light_green", "lime", "yellow", "amber", "orange",
    "deep_orange", "brown", "blue_grey", "grey",
}

# --- enum value maps: short XML name -> LVGL enum constant ------------------
FLEX_FLOW = {
    "row": "LV_FLEX_FLOW_ROW",
    "column": "LV_FLEX_FLOW_COLUMN",
    "row_wrap": "LV_FLEX_FLOW_ROW_WRAP",
    "column_wrap": "LV_FLEX_FLOW_COLUMN_WRAP",
    "row_reverse": "LV_FLEX_FLOW_ROW_REVERSE",
    "column_reverse": "LV_FLEX_FLOW_COLUMN_REVERSE",
}
FLEX_ALIGN = {
    "start": "LV_FLEX_ALIGN_START",
    "end": "LV_FLEX_ALIGN_END",
    "center": "LV_FLEX_ALIGN_CENTER",
    "space_between": "LV_FLEX_ALIGN_SPACE_BETWEEN",
    "space_around": "LV_FLEX_ALIGN_SPACE_AROUND",
    "space_evenly": "LV_FLEX_ALIGN_SPACE_EVENLY",
}
TEXT_ALIGN = {
    "auto": "LV_TEXT_ALIGN_AUTO", "left": "LV_TEXT_ALIGN_LEFT",
    "center": "LV_TEXT_ALIGN_CENTER", "right": "LV_TEXT_ALIGN_RIGHT",
}
TEXT_DECOR = {
    "none": "LV_TEXT_DECOR_NONE", "underline": "LV_TEXT_DECOR_UNDERLINE",
    "strikethrough": "LV_TEXT_DECOR_STRIKETHROUGH",
}
BORDER_SIDE = {
    "none": "LV_BORDER_SIDE_NONE", "bottom": "LV_BORDER_SIDE_BOTTOM",
    "top": "LV_BORDER_SIDE_TOP", "left": "LV_BORDER_SIDE_LEFT",
    "right": "LV_BORDER_SIDE_RIGHT", "full": "LV_BORDER_SIDE_FULL",
    "internal": "LV_BORDER_SIDE_INTERNAL",
}
GRAD_DIR = {
    "none": "LV_GRAD_DIR_NONE", "ver": "LV_GRAD_DIR_VER", "hor": "LV_GRAD_DIR_HOR",
}
BLEND_MODE = {
    "normal": "LV_BLEND_MODE_NORMAL", "additive": "LV_BLEND_MODE_ADDITIVE",
    "subtractive": "LV_BLEND_MODE_SUBTRACTIVE",
    "multiply": "LV_BLEND_MODE_MULTIPLY",
}
BASE_DIR = {
    "ltr": "LV_BASE_DIR_LTR", "rtl": "LV_BASE_DIR_RTL", "auto": "LV_BASE_DIR_AUTO",
}
LAYOUT = {
    "none": "LV_LAYOUT_NONE", "flex": "LV_LAYOUT_FLEX", "grid": "LV_LAYOUT_GRID",
}
ALIGN = {
    "center": "LV_ALIGN_CENTER",
    "top_left": "LV_ALIGN_TOP_LEFT", "top_mid": "LV_ALIGN_TOP_MID",
    "top_right": "LV_ALIGN_TOP_RIGHT",
    "bottom_left": "LV_ALIGN_BOTTOM_LEFT", "bottom_mid": "LV_ALIGN_BOTTOM_MID",
    "bottom_right": "LV_ALIGN_BOTTOM_RIGHT",
    "left_mid": "LV_ALIGN_LEFT_MID", "right_mid": "LV_ALIGN_RIGHT_MID",
    "out_top_left": "LV_ALIGN_OUT_TOP_LEFT", "out_top_mid": "LV_ALIGN_OUT_TOP_MID",
    "out_top_right": "LV_ALIGN_OUT_TOP_RIGHT",
    "out_bottom_left": "LV_ALIGN_OUT_BOTTOM_LEFT",
    "out_bottom_mid": "LV_ALIGN_OUT_BOTTOM_MID",
    "out_bottom_right": "LV_ALIGN_OUT_BOTTOM_RIGHT",
    "out_left_mid": "LV_ALIGN_OUT_LEFT_MID",
    "out_right_mid": "LV_ALIGN_OUT_RIGHT_MID",
}
SCALE_MODE = {
    "horizontal_top": "LV_SCALE_MODE_HORIZONTAL_TOP",
    "horizontal_bottom": "LV_SCALE_MODE_HORIZONTAL_BOTTOM",
    "vertical_left": "LV_SCALE_MODE_VERTICAL_LEFT",
    "vertical_right": "LV_SCALE_MODE_VERTICAL_RIGHT",
    "round_inner": "LV_SCALE_MODE_ROUND_INNER",
    "round_outer": "LV_SCALE_MODE_ROUND_OUTER",
}
KEYBOARD_MODE = {
    "text_lower": "LV_KEYBOARD_MODE_TEXT_LOWER",
    "text_upper": "LV_KEYBOARD_MODE_TEXT_UPPER",
    "special": "LV_KEYBOARD_MODE_SPECIAL",
    "number": "LV_KEYBOARD_MODE_NUMBER",
}
LABEL_LONG = {
    "wrap": "LV_LABEL_LONG_MODE_WRAP", "dots": "LV_LABEL_LONG_MODE_DOTS",
    "scroll": "LV_LABEL_LONG_MODE_SCROLL",
    "scroll_circular": "LV_LABEL_LONG_MODE_SCROLL_CIRCULAR",
    "clip": "LV_LABEL_LONG_MODE_CLIP",
}

# style_<prop> whose value is an enum -> value map (flex_* handled separately).
ENUM_STYLE = {
    "text_align": TEXT_ALIGN,
    "text_decor": TEXT_DECOR,
    "border_side": BORDER_SIDE,
    "bg_grad_dir": GRAD_DIR,
    "blend_mode": BLEND_MODE,
    "base_dir": BASE_DIR,
    "layout": LAYOUT,
    "align": ALIGN,
}
# style_<prop> that take a coordinate (support %, "content").
COORD_STYLE = {
    "width", "height", "min_width", "max_width", "min_height", "max_height",
    "x", "y", "length", "transform_width", "transform_height",
}

# object flag short names (-> LV_OBJ_FLAG_*, prefix with '!' to clear).
OBJ_FLAG = {
    "hidden", "clickable", "click_focusable", "checkable", "scrollable",
    "scroll_elastic", "scroll_momentum", "scroll_one", "scroll_chain_hor",
    "scroll_chain_ver", "scroll_chain", "scroll_on_focus", "scroll_with_arrow",
    "snappable", "press_lock", "event_bubble", "gesture_bubble", "adv_hittest",
    "ignore_layout", "floating", "overflow_visible",
}

# style-selector parts and states.
PART = {
    "main": "LV_PART_MAIN", "scrollbar": "LV_PART_SCROLLBAR",
    "indicator": "LV_PART_INDICATOR", "knob": "LV_PART_KNOB",
    "selected": "LV_PART_SELECTED", "items": "LV_PART_ITEMS",
    "cursor": "LV_PART_CURSOR",
}
STATE = {
    "default": "LV_STATE_DEFAULT", "checked": "LV_STATE_CHECKED",
    "focused": "LV_STATE_FOCUSED", "focus_key": "LV_STATE_FOCUS_KEY",
    "edited": "LV_STATE_EDITED", "hovered": "LV_STATE_HOVERED",
    "pressed": "LV_STATE_PRESSED", "scrolled": "LV_STATE_SCROLLED",
    "disabled": "LV_STATE_DISABLED",
    "user_1": "LV_STATE_USER_1", "user_2": "LV_STATE_USER_2",
    "user_3": "LV_STATE_USER_3", "user_4": "LV_STATE_USER_4",
}


def die(msg):
    sys.stderr.write("gen_ui_xml: %s\n" % msg)
    sys.exit(1)


def c_string(value):
    """Escape an XML text value for a C string literal."""
    out = (value.replace("\\", "\\\\").replace('"', '\\"')
           .replace("\n", "\\n").replace("\t", "\\t").replace("\r", ""))
    return '"%s"' % out


_C_IDENT_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")


def check_ident(name, tag):
    """Reject XML names that are not valid C identifiers (emitted as var names)."""
    if not _C_IDENT_RE.match(name):
        die("invalid name %r on <%s>: must be a valid C identifier" % (name, tag))


def check_child_attrs(tag, a, allowed):
    """Reject unknown attributes on a bespoke child tag (<tab>, <list_*>)."""
    for k in a:
        if k not in allowed:
            die("unknown attribute %r on <%s>" % (k, tag))


def coord(value):
    """Map a width/height to an LVGL size literal (100% -> LV_PCT, content)."""
    value = value.strip()
    if value.endswith("%"):
        return "LV_PCT(%s)" % value[:-1]
    if value == "content":
        return "LV_SIZE_CONTENT"
    return value


def color_expr(value):
    """Palette name (opt. :lighten:N / :darken:N), white/black, hex or #RRGGBB."""
    value = value.strip()
    low = value.lower()
    if low in ("white", "black"):
        return "lv_color_%s()" % low
    parts = low.split(":")
    if parts[0] in PALETTE:
        base = "LV_PALETTE_%s" % parts[0].upper()
        if len(parts) == 3 and parts[1] in ("lighten", "darken"):
            return "lv_palette_%s(%s, %s)" % (parts[1], base, parts[2])
        return "lv_palette_main(%s)" % base
    if value.startswith("#"):
        return "lv_color_hex(0x%s)" % value[1:]
    return "lv_color_hex(%s)" % value


def opa_expr(value):
    value = value.strip().lower()
    if value in ("cover", "transp"):
        return "LV_OPA_%s" % value.upper()
    if value.endswith("%"):
        return str(int(round(255 * int(value[:-1]) / 100)))
    return value


def font_expr(value):
    return "&lv_font_%s" % value.strip()


def is_true(value):
    return value is not None and value.strip().lower() in ("true", "1", "on", "yes")


def enum1(mapping, value, what):
    v = value.strip()
    if v in mapping:
        return mapping[v]
    if v.startswith("LV_"):
        return v
    die("unknown %s value %r" % (what, v))


def enum_flags(mapping, value, what):
    out = []
    for tok in value.replace("|", " ").split():
        out.append(enum1(mapping, tok, what))
    return " | ".join(out)


def selector_expr(tokens):
    """Build an LVGL style selector from part/state tokens (default: 0)."""
    if not tokens:
        return "0"
    out = []
    for tok in tokens:
        if tok in PART:
            out.append(PART[tok])
        elif tok in STATE:
            out.append(STATE[tok])
        else:
            die("unknown style selector %r" % tok)
    return " | ".join(out)


def style_kind(prop):
    if prop == "text_font":
        return "font"
    if prop in ENUM_STYLE:
        return "enum"
    if prop.endswith("_color"):
        return "color"
    if prop.endswith("_opa"):
        return "opa"
    if prop in COORD_STYLE:
        return "coord"
    return "raw"


def style_value(prop, value):
    kind = style_kind(prop)
    if kind == "font":
        return font_expr(value)
    if kind == "enum":
        return enum_flags(ENUM_STYLE[prop], value, "style_%s" % prop)
    if kind == "color":
        return color_expr(value)
    if kind == "opa":
        return opa_expr(value)
    if kind == "coord":
        return coord(value)
    v = value.strip()
    if v.lower() in ("true", "false"):
        return v.lower()
    return v


# --- per-widget value setters (name, attrib) -> list[str] -------------------
def sp_label(em, v, a):
    out = []
    if "text" in a:
        out.append("%slv_label_set_text(%s, %s);" % (TAB, v, c_string(a["text"])))
    if "long_mode" in a:
        out.append("%slv_label_set_long_mode(%s, %s);"
                   % (TAB, v, enum1(LABEL_LONG, a["long_mode"], "long_mode")))
    return out


def sp_checkbox(em, v, a):
    if "text" in a:
        return ["%slv_checkbox_set_text(%s, %s);" % (TAB, v, c_string(a["text"]))]
    return []


def sp_image(em, v, a):
    out = []
    if "src" in a:
        em.img_declares.append(a["src"])
        out.append("%slv_image_set_src(%s, &%s);" % (TAB, v, a["src"]))
    if "rotation" in a:
        out.append("%slv_image_set_rotation(%s, %s);" % (TAB, v, a["rotation"]))
    if "scale" in a:
        out.append("%slv_image_set_scale(%s, %s);" % (TAB, v, a["scale"]))
    return out


def sp_line(em, v, a):
    if "points" not in a:
        return []
    array = "%s_pts" % v
    pts = []
    for p in a["points"].split():
        xy = p.split(",")
        if len(xy) != 2:
            die("<lv_line> points must be 'x,y' pairs, got %r" % p)
        pts.append((xy[0], xy[1]))
    entries = ", ".join("{%s, %s}" % (x.strip(), y.strip()) for x, y in pts)
    em.arrays.append("static const lv_point_precise_t %s[] = {\n%s%s,\n};"
                     % (array, TAB, entries))
    return ["%slv_line_set_points(%s, %s, ARRAY_SIZE(%s));"
            % (TAB, v, array, array)]


def _range(setter, v, a):
    if "min" in a or "max" in a:
        return ["%s%s(%s, %s, %s);"
                % (TAB, setter, v, a.get("min", "0"), a.get("max", "100"))]
    return []


def sp_bar(em, v, a):
    out = _range("lv_bar_set_range", v, a)
    if "value" in a:
        out.append("%slv_bar_set_value(%s, %s, LV_ANIM_OFF);"
                   % (TAB, v, a["value"]))
    return out


def sp_slider(em, v, a):
    out = _range("lv_slider_set_range", v, a)
    if "value" in a:
        out.append("%slv_slider_set_value(%s, %s, LV_ANIM_OFF);"
                   % (TAB, v, a["value"]))
    return out


def _angles(setter, v, value):
    parts = value.split(",")
    if len(parts) != 2:
        die("%s expects 'start,end' angles, got %r" % (setter, value))
    s, e = parts
    return "%s%s(%s, %s, %s);" % (TAB, setter, v, s.strip(), e.strip())


def sp_arc(em, v, a):
    out = _range("lv_arc_set_range", v, a)
    if "bg_angles" in a:
        out.append(_angles("lv_arc_set_bg_angles", v, a["bg_angles"]))
    if "angles" in a:
        out.append(_angles("lv_arc_set_angles", v, a["angles"]))
    if "value" in a:
        out.append("%slv_arc_set_value(%s, %s);" % (TAB, v, a["value"]))
    if "rotation" in a:
        out.append("%slv_arc_set_rotation(%s, %s);" % (TAB, v, a["rotation"]))
    return out


def sp_led(em, v, a):
    out = []
    if "color" in a:
        out.append("%slv_led_set_color(%s, %s);" % (TAB, v, color_expr(a["color"])))
    if "brightness" in a:
        out.append("%slv_led_set_brightness(%s, %s);" % (TAB, v, a["brightness"]))
    if "on" in a:
        fn = "lv_led_on" if is_true(a["on"]) else "lv_led_off"
        out.append("%s%s(%s);" % (TAB, fn, v))
    return out


def sp_dropdown(em, v, a):
    out = []
    if "options" in a:
        opts = a["options"].replace("\\n", "\n")
        out.append("%slv_dropdown_set_options(%s, %s);" % (TAB, v, c_string(opts)))
    if "selected" in a:
        out.append("%slv_dropdown_set_selected(%s, %s);" % (TAB, v, a["selected"]))
    return out


def sp_roller(em, v, a):
    out = []
    if "options" in a:
        opts = a["options"].replace("\\n", "\n")
        mode = ("LV_ROLLER_MODE_INFINITE" if is_true(a.get("infinite"))
                else "LV_ROLLER_MODE_NORMAL")
        out.append("%slv_roller_set_options(%s, %s, %s);"
                   % (TAB, v, c_string(opts), mode))
    if "selected" in a:
        out.append("%slv_roller_set_selected(%s, %s, LV_ANIM_OFF);"
                   % (TAB, v, a["selected"]))
    return out


def sp_textarea(em, v, a):
    out = []
    if "text" in a:
        out.append("%slv_textarea_set_text(%s, %s);" % (TAB, v, c_string(a["text"])))
    if "placeholder" in a:
        out.append("%slv_textarea_set_placeholder_text(%s, %s);"
                   % (TAB, v, c_string(a["placeholder"])))
    if "one_line" in a:
        out.append("%slv_textarea_set_one_line(%s, %s);"
                   % (TAB, v, "true" if is_true(a["one_line"]) else "false"))
    if "password_mode" in a:
        out.append("%slv_textarea_set_password_mode(%s, %s);"
                   % (TAB, v, "true" if is_true(a["password_mode"]) else "false"))
    if "max_length" in a:
        out.append("%slv_textarea_set_max_length(%s, %s);"
                   % (TAB, v, a["max_length"]))
    return out


def sp_spinbox(em, v, a):
    out = []
    if "digit_count" in a or "separator_position" in a:
        out.append("%slv_spinbox_set_digit_format(%s, %s, %s);"
                   % (TAB, v, a.get("digit_count", "5"),
                      a.get("separator_position", "0")))
    out += _range("lv_spinbox_set_range", v, a)
    if "step" in a:
        out.append("%slv_spinbox_set_step(%s, %s);" % (TAB, v, a["step"]))
    if "value" in a:
        out.append("%slv_spinbox_set_value(%s, %s);" % (TAB, v, a["value"]))
    return out


def sp_scale(em, v, a):
    out = []
    if "mode" in a:
        out.append("%slv_scale_set_mode(%s, %s);"
                   % (TAB, v, enum1(SCALE_MODE, a["mode"], "mode")))
    out += _range("lv_scale_set_range", v, a)
    if "total_tick_count" in a:
        out.append("%slv_scale_set_total_tick_count(%s, %s);"
                   % (TAB, v, a["total_tick_count"]))
    if "major_tick_every" in a:
        out.append("%slv_scale_set_major_tick_every(%s, %s);"
                   % (TAB, v, a["major_tick_every"]))
    return out


def sp_calendar(em, v, a):
    if "today" not in a:
        return []
    parts = a["today"].split("-")
    if len(parts) != 3:
        die("<lv_calendar> today must be 'YYYY-MM-DD', got %r" % a["today"])
    y, m, d = parts
    return ["%slv_calendar_set_today_date(%s, %s, %s, %s);"
            % (TAB, v, y.strip(), m.strip(), d.strip())]


def sp_keyboard(em, v, a):
    if "mode" in a:
        return ["%slv_keyboard_set_mode(%s, %s);"
                % (TAB, v, enum1(KEYBOARD_MODE, a["mode"], "mode"))]
    return []


def sp_win(em, v, a):
    if "title" in a:
        return ["%slv_win_add_title(%s, %s);" % (TAB, v, c_string(a["title"]))]
    return []


def sp_buttonmatrix(em, v, a):
    if "map" not in a:
        return []
    array = "%s_map" % v
    items = []
    for tok in a["map"].split():
        items.append('"\\n"' if tok == "\\n" else c_string(tok))
    em.arrays.append('static const char * const %s[] = {%s, ""};'
                     % (array, ", ".join(items)))
    return ["%slv_buttonmatrix_set_map(%s, %s);" % (TAB, v, array)]


SPECIAL = {
    "lv_label": sp_label, "lv_checkbox": sp_checkbox, "lv_image": sp_image,
    "lv_line": sp_line, "lv_bar": sp_bar, "lv_slider": sp_slider, "lv_arc": sp_arc,
    "lv_led": sp_led, "lv_dropdown": sp_dropdown, "lv_roller": sp_roller,
    "lv_textarea": sp_textarea, "lv_spinbox": sp_spinbox, "lv_scale": sp_scale,
    "lv_calendar": sp_calendar, "lv_keyboard": sp_keyboard, "lv_win": sp_win,
    "lv_buttonmatrix": sp_buttonmatrix,
}
# Extra (non-generic, non-style) attributes each widget consumes.
SPECIAL_ATTRS = {
    "lv_label": {"text", "long_mode"},
    "lv_checkbox": {"text"},
    "lv_image": {"src", "rotation", "scale"},
    "lv_line": {"points"},
    "lv_bar": {"value", "min", "max"},
    "lv_slider": {"value", "min", "max"},
    "lv_arc": {"value", "min", "max", "angles", "bg_angles", "rotation"},
    "lv_led": {"color", "brightness", "on"},
    "lv_dropdown": {"options", "selected"},
    "lv_roller": {"options", "selected", "infinite"},
    "lv_textarea": {"text", "placeholder", "one_line", "password_mode",
                    "max_length"},
    "lv_spinbox": {"value", "min", "max", "step", "digit_count",
                   "separator_position"},
    "lv_scale": {"mode", "min", "max", "total_tick_count", "major_tick_every"},
    "lv_calendar": {"today"},
    "lv_keyboard": {"mode"},
    "lv_win": {"title"},
    "lv_buttonmatrix": {"map"},
}


class Emitter:
    def __init__(self):
        self.img_declares = []   # image symbols for LV_IMG_DECLARE
        self.arrays = []         # full "static const ... [] = {...};" declarations
        self.uid = 0             # counter for auto-generated variable names
        self.used_names = {"parent"}  # emitted C var names ("parent" is the root arg)

    def auto_name(self, stem):
        name = "%s_%d" % (stem, self.uid)
        self.uid += 1
        return name

    def use_name(self, name, tag):
        """Register an emitted C variable name, rejecting duplicates early."""
        if name in self.used_names:
            die("duplicate name %r on <%s>: names become C variables and must "
                "be unique" % (name, tag))
        self.used_names.add(name)
        return name

    # -- generic building blocks ------------------------------------------
    def geometry(self, var, a):
        w, h = a.get("width"), a.get("height")
        if w is not None and h is not None:
            return ["%slv_obj_set_size(%s, %s, %s);"
                    % (TAB, var, coord(w), coord(h))]
        if w is not None:
            return ["%slv_obj_set_width(%s, %s);" % (TAB, var, coord(w))]
        if h is not None:
            return ["%slv_obj_set_height(%s, %s);" % (TAB, var, coord(h))]
        return []

    def align(self, var, a):
        if "align" not in a:
            return []
        al = enum1(ALIGN, a["align"], "align")
        if "x" in a or "y" in a:
            return ["%slv_obj_align(%s, %s, %s, %s);"
                    % (TAB, var, al, a.get("x", "0"), a.get("y", "0"))]
        return ["%slv_obj_set_align(%s, %s);" % (TAB, var, al)]

    def flex(self, var, a):
        out = []
        if "style_flex_flow" in a:
            out.append("%slv_obj_set_flex_flow(%s, %s);"
                       % (TAB, var, enum1(FLEX_FLOW, a["style_flex_flow"],
                                          "flex_flow")))
        if ("style_flex_main_place" in a or "style_flex_cross_place" in a
                or "style_flex_track_place" in a):
            main = enum1(FLEX_ALIGN, a.get("style_flex_main_place", "start"),
                         "flex_main_place")
            cross = enum1(FLEX_ALIGN, a.get("style_flex_cross_place", "start"),
                          "flex_cross_place")
            track = enum1(FLEX_ALIGN, a.get("style_flex_track_place", "center"),
                          "flex_track_place")
            out.append("%slv_obj_set_flex_align(%s, %s, %s, %s);"
                       % (TAB, var, main, cross, track))
        return out

    def flags(self, var, a):
        if "flags" not in a:
            return []
        out = []
        for tok in a["flags"].replace(",", " ").split():
            clear = tok.startswith("!")
            name = tok[1:] if clear else tok
            if name not in OBJ_FLAG:
                die("unknown flag %r" % name)
            fn = "lv_obj_remove_flag" if clear else "lv_obj_add_flag"
            out.append("%s%s(%s, LV_OBJ_FLAG_%s);" % (TAB, fn, var, name.upper()))
        return out

    def states(self, var, a):
        toks = []
        if is_true(a.get("checked")):
            toks.append("checked")
        if is_true(a.get("disabled")):
            toks.append("disabled")
        if "states" in a:
            toks += a["states"].replace(",", " ").split()
        out = []
        for tok in toks:
            if tok not in STATE:
                die("unknown state %r" % tok)
            out.append("%slv_obj_add_state(%s, %s);" % (TAB, var, STATE[tok]))
        return out

    def styles(self, var, a, order=None):
        out = []
        keys = order if order is not None else list(a.keys())
        for key in keys:
            if not key.startswith("style_") or key.startswith("style_flex_"):
                continue
            if key not in a:
                continue
            tokens = key[len("style_"):].split("__")
            prop = tokens[0]
            sel = selector_expr(tokens[1:])
            out.append("%slv_obj_set_style_%s(%s, %s, %s);"
                       % (TAB, prop, var, style_value(prop, a[key]), sel))
        return out

    def check_attrs(self, tag, a, extra=()):
        allowed = GENERIC_ATTRS | SPECIAL_ATTRS.get(tag, set()) | set(extra)
        for k in a:
            if k in allowed or k.startswith("style_"):
                continue
            die("unknown attribute %r on <%s>" % (k, tag))

    # -- structure --------------------------------------------------------
    def view(self, view):
        a = view.attrib
        self.check_attrs(view.tag, a, extra=("extends",))
        out = ['%s/* <view extends="%s"> - root container. */'
               % (TAB, a.get("extends", "lv_obj"))]
        out += self.geometry("parent", a)
        out += self.align("parent", a)
        out += self.flex("parent", a)
        out += self.states("parent", a)
        out += self.flags("parent", a)
        out += self.styles("parent", a)
        return out

    def widget(self, elem, idx, parent_var):
        tag = elem.tag
        if tag not in CREATE:
            die("unsupported tag <%s>" % tag)
        a = elem.attrib
        self.check_attrs(tag, a)
        stem = tag[3:] if tag.startswith("lv_") else tag
        if "name" in a:
            check_ident(a["name"], tag)
        name = self.use_name(a.get("name") or self.auto_name(stem), tag)

        out = ['%s/* %d. <%s name="%s"> */' % (TAB, idx, tag, name)]
        out.append("%slv_obj_t *%s = %s(%s);" % (TAB, name, CREATE[tag], parent_var))
        out += self.geometry(name, a)
        out += self.align(name, a)
        fn = SPECIAL.get(tag)
        if fn:
            out += fn(self, name, a)
        out += self.states(name, a)
        out += self.flags(name, a)
        out += self.styles(name, a)

        children = list(elem)
        if children:
            out += self.children(tag, name, children)
        return out

    def children(self, tag, parent_name, children):
        out = []
        if tag == "lv_tabview":
            for i, ch in enumerate(children):
                if ch.tag != "tab":
                    die("<lv_tabview> children must be <tab>, got <%s>" % ch.tag)
                check_child_attrs(ch.tag, ch.attrib, {"text", "name"})
                title = ch.attrib.get("text", ch.attrib.get("name", "Tab"))
                page = self.use_name("%s_t%d" % (parent_name, i), "tab")
                out.append("%slv_obj_t *%s = lv_tabview_add_tab(%s, %s);"
                           % (TAB, page, parent_name, c_string(title)))
                for j, gc in enumerate(list(ch)):
                    out += self.widget(gc, j, page)
            return out
        if tag == "lv_win":
            body = self.use_name("%s_body" % parent_name, "lv_win")
            out.append("%slv_obj_t *%s = lv_win_get_content(%s);"
                       % (TAB, body, parent_name))
            for j, ch in enumerate(children):
                out += self.widget(ch, j, body)
            return out
        if tag == "lv_list":
            for ch in children:
                a = ch.attrib
                if ch.tag == "list_text":
                    check_child_attrs(ch.tag, a, {"text"})
                    out.append("%slv_list_add_text(%s, %s);"
                               % (TAB, parent_name, c_string(a.get("text", ""))))
                elif ch.tag == "list_button":
                    check_child_attrs(ch.tag, a, {"name", "text"})
                    if "name" in a:
                        check_ident(a["name"], ch.tag)
                    nm = self.use_name(a.get("name") or self.auto_name("list_btn"),
                                       ch.tag)
                    out.append("%slv_obj_t *%s = lv_list_add_button(%s, NULL, %s);"
                               % (TAB, nm, parent_name, c_string(a.get("text", ""))))
                else:
                    die("<lv_list> children must be <list_button>/<list_text>, "
                        "got <%s>" % ch.tag)
            return out
        # Any other widget parents its element children directly; LVGL lets any
        # object own children, so nesting works to any depth.
        for j, ch in enumerate(children):
            out += self.widget(ch, j, parent_name)
        return out


HEADER = """\
/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/*
 * GENERATED FILE - do not edit by hand.
 *
 * Produced by scripts/gen_ui_xml.py from ui/screen_main.xml. The Alif LVGL fork
 * has no runtime XML loader (lv_xml_*), so the declarative UI is exported to C
 * offline and compiled. Regenerate after editing the XML:
 *
 *     python scripts/gen_ui_xml.py ui/screen_main.xml src/ui_xml.c
 */

#include <zephyr/sys/util.h>

#include "ui_xml.h"
"""


def generate(xml_path):
    tree = ET.parse(xml_path)
    root = tree.getroot()
    view = root if root.tag == "view" else root.find("view")
    if view is None:
        die("no <view> found in %s" % xml_path)

    em = Emitter()
    body = em.view(view)
    for idx, elem in enumerate(list(view)):
        body.append("")
        body += em.widget(elem, idx, "parent")

    blocks = [HEADER.rstrip("\n")]
    if em.img_declares:
        blocks.append("\n".join("LV_IMG_DECLARE(%s);" % s
                                for s in em.img_declares))
    blocks += em.arrays
    blocks.append("void ui_xml_create(lv_obj_t *parent)\n{\n%s\n}"
                  % "\n".join(body))
    return "\n\n".join(blocks) + "\n"


def main(argv):
    if len(argv) < 2:
        die("usage: gen_ui_xml.py <screen.xml> [out.c]")
    text = generate(argv[1])
    if len(argv) >= 3:
        with open(argv[2], "w") as f:
            f.write(text)
    else:
        sys.stdout.write(text)


if __name__ == "__main__":
    main(sys.argv)
