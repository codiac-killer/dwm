/* See LICENSE file for copyright and license details. */

// Includes ####################################################################
#include <X11/XF86keysym.h>
// #############################################################################

/* appearance */
static const unsigned int borderpx  = 2;        /* border pixel of windows */
static const int CORNER_RADIUS      = 3;        /* radius for rounded corners on clients */
static const unsigned int gappx     = 18;       /* gap pixel between windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static const int showsystray        = 1;        /* 0 means no systray */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const int user_bh            = 35;       /* 0 means that dwm will calculate bar height, >= 1 means dwm will user_bh as bar height */
static const int uline_thickness    = 3;        // thickness of underline of selected tag(s)
static const char *fonts[]          = { "Cantarell:style=Regular:size=10", "JoyPixels:pixelsize=12:antialias=true:autohint=true" };
static const char dmenufont[]       = "Cantarell:style=Regular:size=12";
static const char col_gray1[]       = "#222222";
static const char col_gray2[]       = "#444444";
static const char col_gray3[]       = "#bbbbbb";
static const char col_gray4[]       = "#eeeeee";
static const char col_cyan[]        = "#005577";
static const char col_nord_white[]  = "#ECEFF4";
static const char col_nord_gray[]   = "#2E3440";
static const char col_nord_red[]    = "#BF616A";
static const char col_nord_orange[] = "#d08770";
static const char col_nord_yellow[] = "#ebcb8b";
static const char col_nord_green[]  = "#a3be8c";  // prev 8FBCBB
static const char col_nord_blue[]   = "#5e81ac";
static const char col_nord_lblue[]  = "#81a1c1";
static const char col_nord_purple[] = "#b48ead";
static const char *colors[][3]      = {
    /*                      fg          bg         border */
//  [SchemeNorm]      = { col_gray3, col_gray1, col_gray2 },
//  [SchemeSel]       = { col_gray4, col_cyan,  col_cyan  },
//  [SchemeHid]       = { col_cyan,  col_gray1, col_cyan  },
    [SchemeNorm]      = { col_nord_white, col_gray1, col_gray2 },
    [SchemeSel]       = { col_nord_white, col_gray2, col_cyan },
    [SchemeHid]       = { col_nord_blue, col_gray1, col_nord_blue },
    [SchemeTagText]   = { col_nord_green, col_gray2, col_cyan },
    [SchemeTagUline]  = { col_nord_white, col_nord_green, col_cyan },
    [SchemeCliUline]  = { col_nord_white, col_nord_blue, col_cyan },
    [SchemeStatus]    = { col_gray1, col_nord_red, col_cyan }
};

static const char *rgb_colors[][3]      = {
    { col_nord_red, col_gray1, col_gray2 },
    { col_nord_orange, col_gray1, col_gray2 },
    { col_nord_yellow, col_gray1, col_gray2 },
    { col_nord_green, col_gray1, col_gray2 },
    { col_nord_lblue, col_gray1, col_gray2 },
    { col_nord_purple, col_gray1, col_gray2 },
};

/* tagging */
static const char *tags[] = { "Web", "Code", "Media", "Desktops", "System" };

static const Rule rules[] = {
    /* xprop(1):
     *  WM_CLASS(STRING) = instance, class
     *  WM_NAME(STRING) = title
     */
    /* class            instance    title       tags mask     isfloating   monitor */
    { "Gimp",           NULL,       NULL,       1 << 2,       1,           -1 },
    { "firefox",        NULL,       NULL,       1 << 0,       0,           -1 },
    { "Brave-browser",  NULL,       NULL,       1 << 0,       0,           -1 },
    { "Steam",          NULL,       NULL,       1 << 2,       0,           -1 },
    { "Lutris",         NULL,       NULL,       1 << 2,       0,           -1 },
};

/* layout(s) */
static const float mfact     = 0.5; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */

static const Layout layouts[] = {
    /* symbol     arrange function */
    { "[]=",      tile },    /* first entry is default */
    { "><>",      NULL },    /* no layout function means floating behavior */
    { "[M]",      monocle },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
    { MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
    { MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
    { MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
    { MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[]           = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_gray1, "-nf", col_gray3, "-sb", col_cyan, "-sf", col_gray4, "-c", "-l", "20", NULL };
static const char *roficmd[]            = { "rofi", "-show", "drun", "-modi", "drun", "-theme", "oneDark", NULL };
static const char *termcmd[]            = { "st", NULL };
static const char *fmcmd[]              = { "st", "-e", "sfm", NULL };
static const char *browsercmd[]         = { "qutebrowser", NULL};
// static const char *raise_volumecmd[]     = { "amixer", "-D", "pulse", "sset", "Master", "5%+;", "kill", "-44", "$(pidof", "dwmblocks)", NULL };
// static const char *lower_volumecmd[]     = { "amixer", "-D", "pulse", "sset", "Master", "5%-;", "kill", "-44", "$(pidof", "dwmblocks)", NULL };
// static const char *mute_volumecmd[]      = { "amixer", "-D", "pulse", "sset", "Master", "toggle;", "kill", "-44", "$(pidof", "dwmblocks)", NULL };
// static const char *media_playcmd[]   = { "playerctl", "play-pause", NULL };
// static const char *media_stopcmd[]   = { "playerctl", "stop", NULL };
// static const char *media_nextcmd[]   = { "playerctl", "next", NULL };
// static const char *media_prevcmd[]   = { "playerctl", "previous", NULL };

static Key keys[] = {
    /* modifier                     key        function        argument */
    { MODKEY,                       XK_p,                       spawn,          {.v = dmenucmd } },
    { MODKEY|ControlMask,           XK_Escape,                  spawn,          {.v = roficmd } },
    { MODKEY,                       XK_Return,                  spawn,          {.v = termcmd } },
    { MODKEY,                       XK_e,                       spawn,          {.v = fmcmd } },
    { MODKEY,                       XK_b,                       spawn,          {.v = browsercmd } },
    { MODKEY|ShiftMask,             XK_b,                       togglebar,      {0} },
    { MODKEY,                       XK_j,                       focusstack,     {.i = +1 } },
    { MODKEY,                       XK_k,                       focusstack,     {.i = -1 } },
    { MODKEY,                       XK_Up,                      incnmaster,     {.i = +1 } },
    { MODKEY,                       XK_Down,                    incnmaster,     {.i = -1 } },
    { MODKEY,                       XK_Left,                    setmfact,       {.f = -0.05} },
    { MODKEY,                       XK_Right,                   setmfact,       {.f = +0.05} },
    { MODKEY|ShiftMask,             XK_Return,                  zoom,           {0} },
    { MODKEY,                       XK_Tab,                     view,           {0} },
    { MODKEY,                       XK_x,                       killclient,     {0} },
    // { MODKEY,                       XK_t,                        setlayout,      {.v = &layouts[0]} },
    // { MODKEY,                       XK_f,                        setlayout,      {.v = &layouts[1]} },
    // { MODKEY,                       XK_m,                        setlayout,      {.v = &layouts[2]} },
    // { MODKEY,                       XK_space,                    setlayout,      {0} },
    { MODKEY,                       XK_f,                       togglefloating, {0} },
    { MODKEY,                       XK_0,                       view,           {.ui = ~0 } },
    { MODKEY|ShiftMask,             XK_0,                       tag,            {.ui = ~0 } },
    { MODKEY,                       XK_comma,                   focusmon,       {.i = -1 } },
    { MODKEY,                       XK_period,                  focusmon,       {.i = +1 } },
    { MODKEY|ShiftMask,             XK_comma,                   tagmon,         {.i = -1 } },
    { MODKEY|ShiftMask,             XK_period,                  tagmon,         {.i = +1 } },
    TAGKEYS(                        XK_1,                       0)
    TAGKEYS(                        XK_2,                       1)
    TAGKEYS(                        XK_3,                       2)
    TAGKEYS(                        XK_4,                       3)
    TAGKEYS(                        XK_5,                       4)
    TAGKEYS(                        XK_6,                       5)
    TAGKEYS(                        XK_7,                       6)
    TAGKEYS(                        XK_8,                       7)
    TAGKEYS(                        XK_9,                       8)
    { MODKEY|ShiftMask,             XK_q,                       quit,           {0} },
    // Media Keys
    { 0,                              XF86XK_AudioRaiseVolume,  spawn,          SHCMD("pamixer --allow-boost -i 5; kill -44 $(pidof dwmblocks)") },
    { 0,                              XF86XK_AudioLowerVolume,  spawn,          SHCMD("pamixer --allow-boost -d 5; kill -44 $(pidof dwmblocks)") },
    { 0,                              XF86XK_AudioMute,         spawn,          SHCMD("pamixer -t; kill -44 $(pidof dwmblocks)") },
    { 0,                              XF86XK_AudioPlay,         spawn,          SHCMD("playerctl play-pause") },
    { 0,                              XF86XK_AudioStop,         spawn,          SHCMD("playerctl stop") },
    { 0,                              XF86XK_AudioPrev,         spawn,          SHCMD("playerctl next") },
    { 0,                              XF86XK_AudioNext,         spawn,          SHCMD("playerctl previous") },
    // MPC keys
    { MODKEY,                         XK_F7,                    spawn,          SHCMD("mpc toggle") },
    { MODKEY,                         XK_F8,                    spawn,          SHCMD("mpc stop") },
    { MODKEY,                         XK_F6,                    spawn,          SHCMD("mpc next") },
    { MODKEY,                         XK_F5,                    spawn,          SHCMD("mpc prev") },
    // Print Screen Button:
    { 0,                              XK_Print,                 spawn,          SHCMD("sleep 0.1 && /usr/bin/maim -B --select -s --format png /dev/stdout | xclip -selection clipboard -t image/png -i")},
    { ShiftMask,                      XK_Print,                 spawn,          SHCMD("sleep 0.1 && /usr/bin/maim -B --select ~/Pictures/screenshots/$(date +%F_%H-%M-%S).png")},
  // Lockscreen on command 
    { MODKEY,                         XK_l,                     spawn,          SHCMD("slock") },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
    /* click                event mask      button          function        argument */
    { ClkLtSymbol,          0,              Button1,        setlayout,      {.v = &layouts[0]} },
    { ClkLtSymbol,          0,              Button2,        setlayout,      {.v = &layouts[1]} },
    { ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
    { ClkWinTitle,          0,              Button1,        togglewin,      {0} },
    { ClkWinTitle,          0,              Button2,        zoom,           {0} },
    { ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
    { ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
    // { ClkClientWin,         MODKEY,         Button1,        tilemovemouse,   {0} },
    { ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
    { ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
    { ClkTagBar,            0,              Button1,        view,           {0} },
    { ClkTagBar,            0,              Button3,        toggleview,     {0} },
    { ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
    { ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

