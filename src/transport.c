#include "common.h"
#include "git2/types.h"
#include "git2/transport.h"
#include "git2/net.h"
#include "transport.h"

struct {
	char *prefix;
	git_transport_cb fn;
} transports[] = {
	{"git://", git_transport_git},
	{"http://", git_transport_dummy},
	{"https://", git_transport_dummy},
	{"file://", git_transport_local},
	{"git+ssh://", git_transport_dummy},
	{"ssh+git://", git_transport_dummy},
	{NULL, 0}
};

static git_transport_cb transport_new_fn(const char *url)
{
	int i = 0;

	while (1) {
		if (transports[i].prefix == NULL)
			break;

		if (!strncasecmp(url, transports[i].prefix, strlen(transports[i].prefix)))
			return transports[i].fn;

		++i;
	}

	/*
	 * If we still haven't found the transport, we assume we mean a
	 * local file.
	 * TODO: Parse "example.com:project.git" as an SSH URL
	 */
	return git_transport_local;
}

/**************
 * Public API *
 **************/

int git_transport_dummy(git_transport **GIT_UNUSED(transport))
{
	GIT_UNUSED_ARG(transport);
	return git__throw(GIT_ENOTIMPLEMENTED, "This protocol isn't implemented. Sorry");
}

int git_transport_new(git_transport **out, const char *url)
{
	git_transport_cb fn;
	git_transport *transport;
	int error;

	fn = transport_new_fn(url);

	error = fn(&transport);
	if (error < GIT_SUCCESS)
		return git__rethrow(error, "Failed to create new transport");

	transport->url = git__strdup(url);
	if (transport->url == NULL)
		return GIT_ENOMEM;

	*out = transport;

	return GIT_SUCCESS;
}

int git_transport_connect(git_transport *transport, int direction)
{
	return transport->connect(transport, direction);
}

int git_transport_ls(git_transport *transport, git_headarray *array)
{
	return transport->ls(transport, array);
}

int git_transport_send_wants(struct git_transport *transport, git_headarray *array)
{
	return transport->send_wants(transport, array);
}

int git_transport_send_have(struct git_transport *transport, git_oid *oid)
{
	return transport->send_have(transport, oid);
}

int git_transport_negotiate_fetch(struct git_transport *transport, git_repository *repo, git_headarray *list)
{
	return transport->negotiate_fetch(transport, repo, list);
}

int git_transport_send_flush(struct git_transport *transport)
{
	return transport->send_flush(transport);
}

int git_transport_send_done(struct git_transport *transport)
{
	return transport->send_done(transport);
}

int git_transport_download_pack(char **out, git_transport *transport, git_repository *repo)
{
	return transport->download_pack(out, transport, repo);
}

int git_transport_close(git_transport *transport)
{
	return transport->close(transport);
}

void git_transport_free(git_transport *transport)
{
	transport->free(transport);
}
