#include <libdill.h>
#include <assert.h>
#include <stdio.h>

static coroutine void startTicker(int ch, int64_t ms)
{
	assert(ms > 0);

	int rc;
	int64_t t;
	while (1) {
		rc = msleep(now() + ms);
		if (rc == -1) {
			perror("Could not sleep.");
			return;
		}

		t = now();
		
		rc = chsend(ch, &t, sizeof(t), -1);
		if (rc == -1) {
			perror("Could not send time down channel.");
			return;
		}
	}

}

int TickEvery(int64_t ms)
{
	int ch[2];
	int rc = chmake(ch);
	if (rc != 0) {
		perror("Could not make timer channel.");
		return -1;
	}

	/* An interval of zero means never tick, but we return the channel so the
	   caller can still receive on the channel. In this case it will never 
	   receive any tick. */
	if (ms == 0)
		return ch[0];

	rc = go(startTicker(ch[1], ms));
	if (rc < 0) {
		perror("Could not start ticker coroutine.");
		return -1;
	}

	return ch[0];
}
