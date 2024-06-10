#ifndef ERRSTAT_H
#define ERRSTAT_H

typedef enum ErrStat {
	EOK = 0,
	EBADARGS,
	EDUPFLAG,
	EBADDATE,
	EBADFLAG,
	EBADTIME,
	EEMPFLAG,
	EFILE,
	EOKFINAL,
	EBADID,
	ENOID,
	ETOOMANYARGS,
	EIDNOTFOUND,
	EBADCOMMAND,
	ENOCOMMAND,
} ErrStat;

#endif