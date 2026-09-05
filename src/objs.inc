CFTE_OBJS = \
cfte.$(OEXT) \
s_files.$(OEXT) \
s_string.$(OEXT)

OBJS = \
indent.$(OEXT) \
e_mark.$(OEXT) \
o_modemap.$(OEXT) \
c_desktop.$(OEXT) \
c_bind.$(OEXT) \
c_color.$(OEXT) \
c_config.$(OEXT) \
c_history.$(OEXT) \
c_hilit.$(OEXT) \
c_mode.$(OEXT) \
e_block.$(OEXT) \
e_buffer.$(OEXT) \
e_cmds.$(OEXT) \
e_redraw.$(OEXT) \
e_file.$(OEXT) \
e_fold.$(OEXT) \
e_trans.$(OEXT) \
e_line.$(OEXT) \
e_loadsave.$(OEXT) \
e_regex.$(OEXT) \
e_print.$(OEXT) \
e_search.$(OEXT) \
e_undo.$(OEXT) \
e_tags.$(OEXT) \
g_draw.$(OEXT) \
g_menu.$(OEXT) \
h_ada.$(OEXT) \
h_c.$(OEXT) \
h_fte.$(OEXT) \
h_ipf.$(OEXT) \
h_make.$(OEXT) \
h_pascal.$(OEXT) \
h_perl.$(OEXT) \
h_plain.$(OEXT) \
h_msg.$(OEXT) \
h_rexx.$(OEXT) \
h_sh.$(OEXT) \
h_tex.$(OEXT) \
h_catbs.$(OEXT) \
h_simple.$(OEXT) \
i_complete.$(OEXT) \
i_ascii.$(OEXT) \
i_choice.$(OEXT) \
i_input.$(OEXT) \
i_key.$(OEXT) \
i_search.$(OEXT) \
i_view.$(OEXT) \
i_modelview.$(OEXT) \
i_oview.$(OEXT) \
o_buflist.$(OEXT) \
o_list.$(OEXT) \
o_messages.$(OEXT) \
o_model.$(OEXT) \
o_routine.$(OEXT) \
o_buffer.$(OEXT) \
o_directory.$(OEXT) \
s_files.$(OEXT) \
s_direct.$(OEXT) \
s_util.$(OEXT) \
s_string.$(OEXT) \
view.$(OEXT) \
gui.$(OEXT) \
egui.$(OEXT) \
fte.$(OEXT) \
commands.$(OEXT) \
log.$(OEXT)

UNIXOBJS = \
e_unix.$(OEXT) \
memicmp.$(OEXT)

NOBJS = $(UNIXOBJS) \
g_text.$(OEXT) \
menu_text.$(OEXT) \
con_ncurses.$(OEXT) \
g_unix_pipe.$(OEXT) \
clip_no.$(OEXT) \
g_nodlg.$(OEXT)
