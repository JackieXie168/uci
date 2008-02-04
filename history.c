/*
 * libuci - Library for the Unified Configuration Interface
 * Copyright (C) 2008 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/*
 * This file contains the code for handling uci config history files
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>

int uci_set_savedir(struct uci_context *ctx, const char *dir)
{
	char *sdir;

	UCI_HANDLE_ERR(ctx);
	UCI_ASSERT(ctx, dir != NULL);

	sdir = uci_strdup(ctx, dir);
	if (ctx->savedir != uci_savedir)
		free(ctx->savedir);
	ctx->savedir = sdir;
	return 0;
}

int uci_add_history_path(struct uci_context *ctx, const char *dir)
{
	struct uci_element *e;

	UCI_HANDLE_ERR(ctx);
	UCI_ASSERT(ctx, dir != NULL);
	e = uci_alloc_generic(ctx, UCI_TYPE_PATH, dir, sizeof(struct uci_element));
	uci_list_add(&ctx->history_path, &e->list);

	return 0;
}

static inline void uci_parse_history_tuple(struct uci_context *ctx, char **buf, char **package, char **section, char **option, char **value, bool *delete, bool *rename)
{
	if (**buf == '-') {
		if (delete)
			*delete = true;
		*buf += 1;
	} else if (**buf == '@') {
		if (rename)
			*rename = true;
		*buf += 1;
	}

	UCI_INTERNAL(uci_parse_tuple, ctx, *buf, package, section, option, value);
}
static void uci_parse_history_line(struct uci_context *ctx, struct uci_package *p, char *buf)
{
	bool delete = false;
	bool rename = false;
	char *package = NULL;
	char *section = NULL;
	char *option = NULL;
	char *value = NULL;

	uci_parse_history_tuple(ctx, &buf, &package, &section, &option, &value, &delete, &rename);
	if (!package || (strcmp(package, p->e.name) != 0))
		goto error;
	if (!uci_validate_name(section))
		goto error;
	if (option && !uci_validate_name(option))
		goto error;
	if (rename && !uci_validate_str(value, (option || delete)))
		goto error;

	if (rename)
		UCI_INTERNAL(uci_rename, ctx, p, section, option, value);
	else if (delete)
		UCI_INTERNAL(uci_delete, ctx, p, section, option);
	else
		UCI_INTERNAL(uci_set, ctx, p, section, option, value);

	return;
error:
	UCI_THROW(ctx, UCI_ERR_PARSE);
}

/* returns the number of changes that were successfully parsed */
static int uci_parse_history(struct uci_context *ctx, FILE *stream, struct uci_package *p)
{
	struct uci_parse_context *pctx;
	int changes = 0;

	/* make sure no memory from previous parse attempts is leaked */
	ctx->internal = true;
	uci_cleanup(ctx);

	pctx = (struct uci_parse_context *) uci_malloc(ctx, sizeof(struct uci_parse_context));
	ctx->pctx = pctx;
	pctx->file = stream;

	while (!feof(pctx->file)) {
		uci_getln(ctx, 0);
		if (!pctx->buf[0])
			continue;

		/*
		 * ignore parse errors in single lines, we want to preserve as much
		 * history as possible
		 */
		UCI_TRAP_SAVE(ctx, error);
		uci_parse_history_line(ctx, p, pctx->buf);
		UCI_TRAP_RESTORE(ctx);
		changes++;
error:
		continue;
	}

	/* no error happened, we can get rid of the parser context now */
	ctx->internal = true;
	uci_cleanup(ctx);
	return changes;
}

/* returns the number of changes that were successfully parsed */
static int uci_load_history_file(struct uci_context *ctx, struct uci_package *p, char *filename, FILE **f, bool flush)
{
	FILE *stream = NULL;
	int changes = 0;

	UCI_TRAP_SAVE(ctx, done);
	stream = uci_open_stream(ctx, filename, SEEK_SET, flush, false);
	if (p)
		changes = uci_parse_history(ctx, stream, p);
	UCI_TRAP_RESTORE(ctx);
done:
	if (f)
		*f = stream;
	else if (stream)
		uci_close_stream(stream);
	return changes;
}

/* returns the number of changes that were successfully parsed */
static int uci_load_history(struct uci_context *ctx, struct uci_package *p, bool flush)
{
	struct uci_element *e;
	char *filename = NULL;
	FILE *f = NULL;
	int changes = 0;

	if (!p->confdir)
		return 0;

	uci_foreach_element(&ctx->history_path, e) {
		if ((asprintf(&filename, "%s/%s", e->name, p->e.name) < 0) || !filename)
			UCI_THROW(ctx, UCI_ERR_MEM);

		uci_load_history_file(ctx, p, filename, NULL, false);
		free(filename);
	}

	if ((asprintf(&filename, "%s/%s", ctx->savedir, p->e.name) < 0) || !filename)
		UCI_THROW(ctx, UCI_ERR_MEM);

	changes = uci_load_history_file(ctx, p, filename, &f, flush);
	if (flush && f && (changes > 0)) {
		rewind(f);
		ftruncate(fileno(f), 0);
	}
	if (filename)
		free(filename);
	uci_close_stream(f);
	ctx->errno = 0;
	return changes;
}

static void uci_filter_history(struct uci_context *ctx, const char *name, char *section, char *option)
{
	struct uci_parse_context *pctx;
	struct uci_element *e, *tmp;
	struct uci_list list;
	char *filename = NULL;
	char *p = NULL;
	char *s = NULL;
	char *o = NULL;
	char *v = NULL;
	FILE *f = NULL;

	uci_list_init(&list);
	uci_alloc_parse_context(ctx);
	pctx = ctx->pctx;

	if ((asprintf(&filename, "%s/%s", ctx->savedir, name) < 0) || !filename)
		UCI_THROW(ctx, UCI_ERR_MEM);

	UCI_TRAP_SAVE(ctx, done);
	f = uci_open_stream(ctx, filename, SEEK_SET, true, false);
	pctx->file = f;
	while (!feof(f)) {
		struct uci_element *e;
		char *buf;

		uci_getln(ctx, 0);
		buf = pctx->buf;
		if (!buf[0])
			continue;

		/* NB: need to allocate the element before the call to 
		 * uci_parse_history_tuple, otherwise the original string 
		 * gets modified before it is saved */
		e = uci_alloc_generic(ctx, UCI_TYPE_HISTORY, pctx->buf, sizeof(struct uci_element));
		uci_list_add(&list, &e->list);

		uci_parse_history_tuple(ctx, &buf, &p, &s, &o, &v, NULL, NULL);
		if (section) {
			if (!s || (strcmp(section, s) != 0))
				continue;
		}
		if (option) {
			if (!o || (strcmp(option, o) != 0))
				continue;
		}
		/* match, drop this element again */
		uci_free_element(e);
	}

	/* rebuild the history file */
	rewind(f);
	ftruncate(fileno(f), 0);
	uci_foreach_element_safe(&list, tmp, e) {
		fprintf(f, "%s\n", e->name);
		uci_free_element(e);
	}
	UCI_TRAP_RESTORE(ctx);

done:
	if (filename)
		free(filename);
	uci_close_stream(f);
	uci_foreach_element_safe(&list, tmp, e) {
		uci_free_element(e);
	}
	ctx->internal = true;
	uci_cleanup(ctx);
}

int uci_revert(struct uci_context *ctx, struct uci_package **pkg, char *section, char *option)
{
	struct uci_package *p;
	char *name = NULL;

	UCI_HANDLE_ERR(ctx);
	UCI_ASSERT(ctx, pkg != NULL);
	p = *pkg;
	UCI_ASSERT(ctx, p != NULL);
	UCI_ASSERT(ctx, p->confdir);

	/* 
	 * - flush unwritten changes
	 * - save the package name
	 * - unload the package
	 * - filter the history
	 * - reload the package
	 */
	UCI_TRAP_SAVE(ctx, error);
	UCI_INTERNAL(uci_save, ctx, p);
	name = uci_strdup(ctx, p->e.name);

	*pkg = NULL;
	uci_free_package(&p);
	uci_filter_history(ctx, name, section, option);

	UCI_INTERNAL(uci_load, ctx, name, &p);
	UCI_TRAP_RESTORE(ctx);
	ctx->errno = 0;

error:
	if (name)
		free(name);
	if (ctx->errno)
		UCI_THROW(ctx, ctx->errno);
	return 0;
}

int uci_save(struct uci_context *ctx, struct uci_package *p)
{
	FILE *f = NULL;
	char *filename = NULL;
	struct uci_element *e, *tmp;

	UCI_HANDLE_ERR(ctx);
	UCI_ASSERT(ctx, p != NULL);

	/* 
	 * if the config file was outside of the /etc/config path,
	 * don't save the history to a file, update the real file
	 * directly.
	 * does not modify the uci_package pointer
	 */
	if (!p->confdir)
		return uci_commit(ctx, &p, false);

	if (uci_list_empty(&p->history))
		return 0;

	if ((asprintf(&filename, "%s/%s", ctx->savedir, p->e.name) < 0) || !filename)
		UCI_THROW(ctx, UCI_ERR_MEM);

	ctx->errno = 0;
	UCI_TRAP_SAVE(ctx, done);
	f = uci_open_stream(ctx, filename, SEEK_END, true, true);
	UCI_TRAP_RESTORE(ctx);

	uci_foreach_element_safe(&p->history, tmp, e) {
		struct uci_history *h = uci_to_history(e);

		if (h->cmd == UCI_CMD_REMOVE)
			fprintf(f, "-");
		else if (h->cmd == UCI_CMD_RENAME)
			fprintf(f, "@");

		fprintf(f, "%s.%s", p->e.name, h->section);
		if (e->name)
			fprintf(f, ".%s", e->name);

		if (h->cmd == UCI_CMD_REMOVE)
			fprintf(f, "\n");
		else
			fprintf(f, "=%s\n", h->value);
		uci_free_history(h);
	}

done:
	uci_close_stream(f);
	if (filename)
		free(filename);
	if (ctx->errno)
		UCI_THROW(ctx, ctx->errno);

	return 0;
}


