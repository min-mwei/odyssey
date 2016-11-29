#ifndef OD_CLIENT_H_
#define OD_CLIENT_H_

/*
 * odissey.
 *
 * PostgreSQL connection pooler and request router.
*/

typedef struct od_client_t od_client_t;

struct od_client_t {
	mm_io_t         io;
	so_bestartup_t  startup;
	so_key_t        key;
	so_stream_t     stream;
	od_server_t    *server;
	void           *pooler;
	uint64_t        id;
	od_list_t       link;
};

static inline void
od_clientinit(od_client_t *c)
{
	c->id = 0;
	c->io = NULL;
	c->server = NULL;
	c->pooler = NULL;
	so_bestartup_init(&c->startup);
	so_keyinit(&c->key);
	so_stream_init(&c->stream);
	od_listinit(&c->link);
}

static inline od_client_t*
od_clientalloc(void)
{
	od_client_t *c = malloc(sizeof(*c));
	if (c == NULL)
		return NULL;
	od_clientinit(c);
	return c;
}

static inline void
od_clientfree(od_client_t *c)
{
	so_bestartup_free(&c->startup);
	so_stream_free(&c->stream);
	free(c);
}

#endif
