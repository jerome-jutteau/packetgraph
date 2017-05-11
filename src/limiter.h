/* Copyright 2017 Outscale SAS
 *
 * This file is part of Packetgraph.
 *
 * Packetgraph is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as published
 * by the Free Software Foundation.
 *
 * Packetgraph is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Packetgraph.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _PG_LIMITER_H
#define _PG_LIMITER_H

struct pg_limiter {
	uint64_t next_date;
	uint64_t cycles_per_poll;
	uint64_t mtu;
	uint64_t need_update;
};

void pg_limiter_init(struct pg_limiter *l, uint16_t mtu);

/* Update poll limiter
 *
 * @l		limiter structure
 * @mut		current mtu size or mean
 * @burst_s	number of packets per bursts
 * @bps		bits par seconds to achieve
 */
void pg_limiter_update(struct pg_limiter *l,
		       uint16_t mtu,
		       uint16_t burst_s,
		       uint64_t bps);

enum pg_limiter_action {
	PG_LIMITER_WAIT = 0,   /* do not poll yet */
	PG_LIMITER_POLL = 1,   /* you can poll */
	PG_LIMITER_UPDATE = 2, /* you can call pg_limiter_update */
};

/* Check if it's time to poll */
enum pg_limiter_action pg_limiter_go(struct pg_limiter *l);

#endif /* _PG_LIMITER_H */
