#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "mruby.h"
#include "mruby/class.h"
#include "mruby/data.h"
#include "mruby/string.h"
#include "mruby/array.h"
#include "mruby/variable.h"
#include "mruby/hash.h"

#include "Scintilla.h"

#include "ScintillaTermbox2.h"

#define DONE mrb_gc_arena_restore(mrb, 0)

typedef void Scintilla;

static mrb_state *scmrb;

struct mrb_scintilla_data {
  Scintilla *view;
  mrb_value view_obj;
  bool has_callback;
  struct mrb_scintilla_data *next;
};

struct mrb_scintilla_doc_data {
  sptr_t pdoc;
};

static struct mrb_scintilla_data *scintilla_list = NULL;

static void scintilla_termbox_free(mrb_state *mrb, void *ptr) {
// fprintf(stderr, "scintilla_curses_free %p\n", ptr);
  if (ptr != NULL) {
    scintilla_delete((Scintilla *)ptr);
  }
}

const static struct mrb_data_type mrb_scintilla_termbox_type = { "ScintillaTermbox2", scintilla_termbox_free };
const static struct mrb_data_type mrb_document_type = { "Document", mrb_free };

void scnotification(Scintilla *view, int msg, SCNotification *n, void *userdata) {
  struct mrb_scintilla_data *scdata = scintilla_list;
  mrb_value callback, scn;

  while (scdata != NULL) {
    if (scdata->view == view) {
      break;
    }
    scdata = scdata->next;
  }
  if (scdata == NULL) {
    fprintf(stderr, "scdata = NULL\n");
    return;
  }

  if (scdata->has_callback == TRUE) {
    callback = mrb_iv_get(scmrb, scdata->view_obj, mrb_intern_cstr(scmrb, "notification"));
    if (mrb_nil_p(callback)) {
	    return;
    }
    scn = mrb_hash_new(scmrb);
    mrb_hash_set(scmrb, scn, mrb_str_new_cstr(scmrb, "code"), mrb_fixnum_value(n->nmhdr.code));
    // Sci_Position position
    mrb_hash_set(scmrb, scn, mrb_str_new_cstr(scmrb, "position"), mrb_fixnum_value(n->position));
    // int ch
    mrb_hash_set(scmrb, scn, mrb_str_new_cstr(scmrb, "ch"), mrb_fixnum_value(n->ch));
    // int modifiers
    mrb_hash_set(scmrb, scn, mrb_str_new_cstr(scmrb, "modifiers"), mrb_fixnum_value(n->modifiers));
    // int modificationType
    mrb_hash_set(scmrb, scn, mrb_str_new_cstr(scmrb, "modification_type"),
      mrb_fixnum_value(n->modificationType));
    // const char *text
    if (n->text == NULL) {
      mrb_hash_set(scmrb, scn, mrb_str_new_cstr(scmrb, "text"), mrb_nil_value());
    } else if (n->nmhdr.code == SCN_MODIFIED) {
      mrb_hash_set(scmrb, scn, mrb_str_new_cstr(scmrb, "text"),
        mrb_str_new(scmrb, n->text, n->length));
    } else {
      mrb_hash_set(scmrb, scn, mrb_str_new_cstr(scmrb, "text"),
        mrb_str_new_cstr(scmrb, n->text));
    }
    // Sci_Position length
    mrb_hash_set(scmrb, scn, mrb_str_new_cstr(scmrb, "length"), mrb_fixnum_value(n->length));
    // Sci_Position linesAdded
    mrb_hash_set(scmrb, scn, mrb_str_new_cstr(scmrb, "lines_added"), mrb_fixnum_value(n->linesAdded));
    // int message
    mrb_hash_set(scmrb, scn, mrb_str_new_cstr(scmrb, "message"), mrb_fixnum_value(n->message));
    // uptr_t wParam
    mrb_hash_set(scmrb, scn, mrb_str_new_cstr(scmrb, "w_param"), mrb_fixnum_value(n->wParam));
    // sptr_t lParam
    mrb_hash_set(scmrb, scn, mrb_str_new_cstr(scmrb, "l_param"), mrb_fixnum_value(n->lParam));
    // Sci_Position line
    mrb_hash_set(scmrb, scn, mrb_str_new_cstr(scmrb, "line"), mrb_fixnum_value(n->line));
    // int foldLevelNow
    mrb_hash_set(scmrb, scn, mrb_str_new_cstr(scmrb, "fold_level_now"),
      mrb_fixnum_value(n->foldLevelNow));
    // int foldLevelPrev
    mrb_hash_set(scmrb, scn, mrb_str_new_cstr(scmrb, "fold_level_prev"),
      mrb_fixnum_value(n->foldLevelPrev));
    // int margin
    mrb_hash_set(scmrb, scn, mrb_str_new_cstr(scmrb, "margin"), mrb_fixnum_value(n->margin));
    // int listType
    mrb_hash_set(scmrb, scn, mrb_str_new_cstr(scmrb, "list_type"), mrb_fixnum_value(n->listType));
    // int x
    mrb_hash_set(scmrb, scn, mrb_str_new_cstr(scmrb, "x"), mrb_fixnum_value(n->x));
    // int y
    mrb_hash_set(scmrb, scn, mrb_str_new_cstr(scmrb, "y"), mrb_fixnum_value(n->y));
    // int token
    mrb_hash_set(scmrb, scn, mrb_str_new_cstr(scmrb, "token"), mrb_fixnum_value(n->token));
    // int annotationLinesAdded
    mrb_hash_set(scmrb, scn, mrb_str_new_cstr(scmrb, "annotation_lines_added"),
      mrb_fixnum_value(n->annotationLinesAdded));
    // int updated
    mrb_hash_set(scmrb, scn, mrb_str_new_cstr(scmrb, "updated"), mrb_fixnum_value(n->updated));
    // listCompletionMethod
    mrb_hash_set(scmrb, scn, mrb_str_new_cstr(scmrb, "list_completion_method"),
                 mrb_fixnum_value(n->listCompletionMethod));
    // int characterSource
    mrb_hash_set(scmrb, scn, mrb_str_new_cstr(scmrb, "character_source"),
                 mrb_fixnum_value(n->characterSource));

    mrb_yield(scmrb, callback, scn);
  }
  /*
    struct SCNotification *scn = (struct SCNotification *)lParam;
    printw("SCNotification received: %i", scn->nmhdr.code);
  */
}


static mrb_value
mrb_scintilla_termbox_initialize(mrb_state *mrb, mrb_value self)
{
  Scintilla *sci = scintilla_new(scnotification, mrb);
  mrb_value callback;
  //     struct mrb_scintilla_data *scdata = mrb_malloc(mrb, sizeof(struct mrb_scintilla_data));
  struct mrb_scintilla_data *scdata = (struct mrb_scintilla_data *)malloc(sizeof(struct mrb_scintilla_data));
  struct mrb_scintilla_data *tmp;

  mrb_get_args(mrb, "|&", &callback);
  DATA_TYPE(self) = &mrb_scintilla_termbox_type;
  DATA_PTR(self) = sci;
  scdata->view = sci;
  scdata->view_obj = self;
  if (!mrb_nil_p(callback)) {
    mrb_iv_set(mrb, self, mrb_intern_cstr(mrb, "notification"), callback);
    scdata->has_callback = TRUE;
  } else {
    scdata->has_callback = FALSE;
  }
  scdata->next = NULL;

  if (scintilla_list == NULL) {
    scintilla_list = scdata;
  } else {
    tmp = scintilla_list;
    while (tmp->next != NULL) {
      tmp = tmp->next;
    }
    tmp->next = scdata;
  }
  return self;
}

/* scintilla_delete (sci) */
static mrb_value
mrb_scintilla_termbox_delete(mrb_state *mrb, mrb_value self)
{
  Scintilla *sci;
  sci = (Scintilla *)DATA_PTR(self);
  scintilla_delete(sci);
  DATA_PTR(self) = NULL;
  return mrb_nil_value();
}

/* scintilla_get_clipboard (sci, buffer) */
/*
static mrb_value
mrb_scintilla_termbox_get_clipboard(mrb_state *mrb, mrb_value self)
{
  Scintilla *sci;
  char *buffer = NULL;
  int len;

  sci = (Scintilla *)DATA_PTR(self);
  buffer = scintilla_get_clipboard(sci, &len);
  return mrb_str_new(mrb, buffer, len);
}
*/
/*
static mrb_value
mrb_scintilla_curses_noutrefresh(mrb_state *mrb, mrb_value self)
{
  Scintilla *sci = (Scintilla *)DATA_PTR(self);

  scintilla_noutrefresh(sci);
  return mrb_nil_value();
}
*/
static mrb_value
mrb_scintilla_termbox_refresh(mrb_state *mrb, mrb_value self)
{
  Scintilla *sci = (Scintilla *)DATA_PTR(self);

  scintilla_refresh(sci);
  return mrb_nil_value();
}

/*scintilla_send_key (sci, key, shift, ctrl, alt) */
static mrb_value
mrb_scintilla_termbox_send_key(mrb_state *mrb, mrb_value self)
{
  Scintilla *sci = (Scintilla *)DATA_PTR(self);
  mrb_int key;
  mrb_bool shift, ctrl, alt;

  mrb_get_args(mrb, "ibbb", &key, &shift, &ctrl, &alt);
  scintilla_send_key(sci, key, shift, ctrl, alt);
  return mrb_nil_value();
}

/* scintilla_update_cursor(sci) */
/*
static mrb_value
mrb_scintilla_termbox_update_cursor(mrb_state *mrb, mrb_value self)
{
  Scintilla *sci = (Scintilla *)DATA_PTR(self);

  scintilla_update_cursor(sci);
  return mrb_nil_value();
}
*/

static mrb_value
mrb_scintilla_termbox_send_message(mrb_state *mrb, mrb_value self)
{
  Scintilla *sci = (Scintilla *)DATA_PTR(self);
  sptr_t ret;
  mrb_int i_message, argc;
  uptr_t w_param = 0;
  sptr_t l_param = 0;
  mrb_value w_param_obj, l_param_obj;

  argc = mrb_get_args(mrb, "i|oo", &i_message, &w_param_obj, &l_param_obj);
  if (i_message < SCI_START) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid message");
    return mrb_nil_value();
  }
  if (argc > 1) {
    switch(mrb_type(w_param_obj)) {
    case MRB_TT_FIXNUM:
      w_param = (uptr_t)mrb_fixnum(w_param_obj);
      break;
    case MRB_TT_STRING:
      w_param = (uptr_t)mrb_string_value_ptr(mrb, w_param_obj);
      break;
    case MRB_TT_TRUE:
      w_param = TRUE;
      break;
    case MRB_TT_FALSE:
      w_param = FALSE;
      break;
    case MRB_TT_UNDEF:
      w_param = 0;
      break;
    default:
      mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid parameter");
      return mrb_nil_value();
    }
  }
  if (argc > 2) {
    switch(mrb_type(l_param_obj)) {
    case MRB_TT_FIXNUM:
      l_param = (sptr_t)mrb_fixnum(l_param_obj);
      break;
    case MRB_TT_STRING:
      l_param = (sptr_t)mrb_string_value_ptr(mrb, l_param_obj);
      break;
    case MRB_TT_TRUE:
      l_param = TRUE;
      break;
    case MRB_TT_FALSE:
      l_param = FALSE;
      break;
    case MRB_TT_UNDEF:
      l_param = 0;
      break;
    default:
      mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid parameter");
      return mrb_nil_value();
    }
  }

  ret = scintilla_send_message(sci, i_message, w_param, l_param);
  /*
    if (i_message == SCI_SETPROPERTY) {
    fprintf(stderr, "w_param = %s, l_param = %s\n", w_param, l_param);
    }
  */
  return mrb_fixnum_value(ret);
}

static mrb_value
mrb_scintilla_termbox_send_message_get_str(mrb_state *mrb, mrb_value self)
{
  Scintilla *sci = (Scintilla *)DATA_PTR(self);
  mrb_value w_param_obj;
  uptr_t w_param = 0;
  char *value = NULL;
  mrb_int i_message, len, argc;
  argc = mrb_get_args(mrb, "i|o", &i_message, &w_param_obj);
  if (argc > 1) {
    switch(mrb_type(w_param_obj)) {
      case MRB_TT_FIXNUM:
      w_param = (uptr_t)mrb_fixnum(w_param_obj);
      break;
      case MRB_TT_STRING:
      w_param = (uptr_t)mrb_string_value_ptr(mrb, w_param_obj);
      break;
      case MRB_TT_TRUE:
      w_param = TRUE;
      break;
      case MRB_TT_FALSE:
      w_param = FALSE;
      break;
      case MRB_TT_UNDEF:
      w_param = 0;
      break;
      default:
      mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid parameter");
      return mrb_nil_value();
    }
  }

  len = scintilla_send_message(sci, i_message, w_param, (sptr_t)NULL);

  value = (char *)mrb_malloc(mrb, sizeof(char)*len + 1);
  len = scintilla_send_message(sci, i_message, w_param, (sptr_t)value);
  value[len] = '\0';
  return mrb_str_new_cstr(mrb, value);
}

static mrb_value
mrb_scintilla_termbox_send_message_get_text(mrb_state *mrb, mrb_value self)
{
  Scintilla *sci = (Scintilla *)DATA_PTR(self);
  char *text = NULL;
  mrb_int i_message, nlen;

  mrb_get_args(mrb, "ii", &i_message, &nlen);
  text = (char *)mrb_malloc(mrb, sizeof(char) * nlen + 1);
  scintilla_send_message(sci, i_message, (uptr_t)nlen, (sptr_t)text);
  return mrb_str_new_cstr(mrb, text);
}

static mrb_value
mrb_scintilla_termbox_send_message_get_text_range(mrb_state *mrb, mrb_value self)
{
  Scintilla *sci = (Scintilla *)DATA_PTR(self);

  mrb_int i_message, cp_min, cp_max;
  struct Sci_TextRange *tr = (struct Sci_TextRange *)mrb_malloc(mrb, sizeof(struct Sci_TextRange));

  mrb_get_args(mrb, "iii", &i_message, &cp_min, &cp_max);
  tr->chrg.cpMin = cp_min;
  tr->chrg.cpMax = cp_max;
  tr->lpstrText = (char *)mrb_malloc(mrb, sizeof(char)*(cp_max - cp_min + 2));

  scintilla_send_message(sci, i_message, 0, (sptr_t)tr);
  return mrb_str_new_cstr(mrb, tr->lpstrText);
}


static mrb_value
mrb_scintilla_termbox_send_message_get_curline(mrb_state *mrb, mrb_value self)
{
  Scintilla *sci = (Scintilla *)DATA_PTR(self);
  char *text = NULL;
  mrb_int len, pos;
  mrb_value ret_a = mrb_ary_new(mrb);

  len = scintilla_send_message(sci, SCI_GETCURLINE, (uptr_t)0, (sptr_t)0);
  text = (char *)mrb_malloc(mrb, sizeof(char) * len + 1);
  pos = scintilla_send_message(sci, SCI_GETCURLINE, (uptr_t)len, (sptr_t)text);
  mrb_ary_push(mrb, ret_a, mrb_str_new_cstr(mrb, text));
  mrb_ary_push(mrb, ret_a, mrb_fixnum_value(pos));
  return ret_a;
}

static mrb_value
mrb_scintilla_termbox_send_message_get_line(mrb_state *mrb, mrb_value self)
{
  Scintilla *sci = (Scintilla *)DATA_PTR(self);
  char *text = NULL;
  mrb_int line, len;

  mrb_get_args(mrb, "i", &line);
  len = scintilla_send_message(sci, SCI_LINELENGTH, (uptr_t)line, (sptr_t)0);
  text = (char *)mrb_malloc(mrb, sizeof(char) * len + 1);
  scintilla_send_message(sci, SCI_GETLINE, (uptr_t)line, (sptr_t)text);
  text[len] = '\0';

  return mrb_str_new_cstr(mrb, text);
}

static mrb_value
mrb_scintilla_termbox_send_message_get_docpointer(mrb_state *mrb, mrb_value self)
{
  Scintilla *sci = (Scintilla *)DATA_PTR(self);
  sptr_t pdoc;
  mrb_int argc, i_message, w_param, l_param;

  argc = mrb_get_args(mrb, "i|ii", &i_message, &w_param, &l_param);
  if (argc == 1) {
    w_param = 0;
    l_param = 0;
  }
  struct mrb_scintilla_doc_data *doc = (struct mrb_scintilla_doc_data *)mrb_malloc(mrb, sizeof(struct mrb_scintilla_doc_data));

  pdoc = scintilla_send_message(sci, i_message, (uptr_t)w_param, (sptr_t)l_param);
  if (pdoc == 0) {
    return mrb_nil_value();
  } else {
    doc->pdoc = pdoc;
    return mrb_obj_value(mrb_data_object_alloc(mrb, mrb_class_get_under(mrb, mrb_module_get(mrb, "Scintilla"), "Document"), doc, &mrb_document_type));
  }
}

static mrb_value
mrb_scintilla_termbox_send_message_set_docpointer(mrb_state *mrb, mrb_value self)
{
  Scintilla *sci = (Scintilla *)DATA_PTR(self);
  struct mrb_scintilla_doc_data *doc;
  mrb_int i_message;
  mrb_int ret;
  mrb_value doc_obj;

  mrb_get_args(mrb, "io", &i_message, &doc_obj);
  if (mrb_nil_p(doc_obj) || mrb_integer_p(doc_obj)) {
    ret = scintilla_send_message(sci, i_message, 0, (sptr_t)0);
  } else {
    doc = (struct mrb_scintilla_doc_data *)DATA_PTR(doc_obj);
    ret = scintilla_send_message(sci, i_message, 0, doc->pdoc);
  }
  return mrb_fixnum_value(ret);
}

static mrb_value
mrb_scintilla_termbox_send_message_set_pointer(mrb_state *mrb, mrb_value self)
{
  Scintilla *sci = (Scintilla *)DATA_PTR(self);
  mrb_int i_message;
  mrb_int ret;
  mrb_value cptr_obj;

  mrb_get_args(mrb, "io", &i_message, &cptr_obj);
  if (mrb_nil_p(cptr_obj) || mrb_integer_p(cptr_obj)) {
    ret = scintilla_send_message(sci, i_message, 0, (sptr_t)0);
  } else if (mrb_cptr_p(cptr_obj)){

    ret = scintilla_send_message(sci, i_message, 0, (sptr_t)mrb_cptr(cptr_obj));
  } else {
    return mrb_nil_value();
  }
  return mrb_fixnum_value(ret);
}

static mrb_value
mrb_scintilla_termbox_send_mouse(mrb_state *mrb, mrb_value self)
{
  Scintilla *sci = (Scintilla *)DATA_PTR(self);
  mrb_int event, button, y, x;
  mrb_bool shift, ctrl, alt, ret;

  mrb_get_args(mrb, "iiiibbb", &event, &button, &y, &x, &shift, &ctrl, &alt);
  ret = scintilla_send_mouse(sci, event, button, y, x, shift, ctrl, alt);

  return (ret == TRUE)? mrb_true_value() : mrb_false_value();
}

/* scintilla_get_clipboard (sci, buffer) */
static mrb_value
mrb_scintilla_termbox_get_clipboard(mrb_state *mrb, mrb_value self)
{
  Scintilla *sci;
  char *buffer = NULL;
  int len;

  sci = (Scintilla *)DATA_PTR(self);
  buffer = scintilla_get_clipboard(sci, &len);
  return mrb_str_new(mrb, buffer, len);
}

static mrb_value
mrb_scintilla_termbox_resize(mrb_state *mrb, mrb_value self)
{
  Scintilla *sci = (Scintilla *)DATA_PTR(self);
  mrb_int width, height;

  mrb_get_args(mrb, "ii", &width, &height);
  scintilla_resize(sci, width, height);
  return mrb_nil_value();
}

static mrb_value
mrb_scintilla_termbox_move(mrb_state *mrb, mrb_value self)
{
  Scintilla *sci = (Scintilla *)DATA_PTR(self);
  mrb_int new_x, new_y;

  mrb_get_args(mrb, "ii", &new_x, &new_y);
  scintilla_move(sci, new_x, new_y);
  return mrb_nil_value();
}

void
mrb_mruby_scintilla_termbox2_gem_init(mrb_state* mrb)
{
  struct RClass *sci, *scim, *doc;

  scim = mrb_module_get(mrb, "Scintilla");

  mrb_define_const(mrb, scim, "PLATFORM", mrb_symbol_value(mrb_intern_cstr(mrb, "TERMBOX")));

  sci = mrb_define_class_under(mrb, scim, "ScintillaTermbox2", mrb_class_get_under(mrb, scim, "ScintillaBase"));
  MRB_SET_INSTANCE_TT(sci, MRB_TT_DATA);

  doc = mrb_define_class_under(mrb, scim, "Document", mrb->object_class);
  MRB_SET_INSTANCE_TT(doc, MRB_TT_DATA);

  mrb_define_method(mrb, sci, "initialize", mrb_scintilla_termbox_initialize, MRB_ARGS_OPT(1));
  mrb_define_method(mrb, sci, "delete", mrb_scintilla_termbox_delete, MRB_ARGS_NONE());
  mrb_define_method(mrb, sci, "refresh", mrb_scintilla_termbox_refresh, MRB_ARGS_NONE());
  mrb_define_method(mrb, sci, "send_key", mrb_scintilla_termbox_send_key, MRB_ARGS_REQ(4));

  mrb_define_method(mrb, sci, "send_message", mrb_scintilla_termbox_send_message, MRB_ARGS_ARG(1, 2));
  mrb_define_method(mrb, sci, "send_message_get_str", mrb_scintilla_termbox_send_message_get_str,
    MRB_ARGS_ARG(1, 1));
  mrb_define_method(mrb, sci, "send_message_get_text", mrb_scintilla_termbox_send_message_get_text,
    MRB_ARGS_REQ(2));
  mrb_define_method(mrb, sci, "send_message_get_text_range", mrb_scintilla_termbox_send_message_get_text_range,
    MRB_ARGS_REQ(3));
  mrb_define_method(mrb, sci, "send_message_get_curline", mrb_scintilla_termbox_send_message_get_curline, MRB_ARGS_NONE());
  mrb_define_method(mrb, sci, "send_message_get_line", mrb_scintilla_termbox_send_message_get_line, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, sci, "send_message_get_docpointer", mrb_scintilla_termbox_send_message_get_docpointer,
    MRB_ARGS_ARG(1, 2));
  mrb_define_method(mrb, sci, "send_message_set_docpointer", mrb_scintilla_termbox_send_message_set_docpointer,
    MRB_ARGS_REQ(2));
  mrb_define_method(mrb, sci, "send_message_set_pointer", mrb_scintilla_termbox_send_message_set_pointer, MRB_ARGS_REQ(2));

  mrb_define_method(mrb, sci, "send_mouse", mrb_scintilla_termbox_send_mouse, MRB_ARGS_REQ(7));
  mrb_define_method(mrb, sci, "get_clipboard", mrb_scintilla_termbox_get_clipboard, MRB_ARGS_NONE());

  mrb_define_method(mrb, sci, "resize", mrb_scintilla_termbox_resize, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, sci, "move", mrb_scintilla_termbox_move, MRB_ARGS_REQ(2));

  mrb_define_const(mrb, scim, "SCM_PRESS", mrb_fixnum_value(SCM_PRESS));
  mrb_define_const(mrb, scim, "SCM_DRAG", mrb_fixnum_value(SCM_DRAG));
  mrb_define_const(mrb, scim, "SCM_RELEASE", mrb_fixnum_value(SCM_RELEASE));

  scmrb = mrb;

  DONE;
}

void
mrb_mruby_scintilla_termbox2_gem_final(mrb_state* mrb)
{
}
