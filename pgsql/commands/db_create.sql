CREATE USER collector WITH
  LOGIN
  NOSUPERUSER
  INHERIT
  NOCREATEDB
  NOCREATEROLE
  NOREPLICATION;

ALTER USER collector
  PASSWORD 'dbPassword';

CREATE DATABASE collector
  WITH 
  OWNER = collector
  ENCODING = 'UTF8'
  LC_COLLATE = 'C'
  LC_CTYPE = 'C'
  TABLESPACE = pg_default
  CONNECTION LIMIT = -1;

ALTER DATABASE collector
  SET "TimeZone" TO 'UTC';

\connect collector;

CREATE TABLE public.statistics
(
  user_key text COLLATE pg_catalog."C" NOT NULL,
  time_stamp timestamp without time zone NOT NULL,
  cpu_usage integer,
  memory_usage integer,
  process_count integer
)
WITH (
  OIDS = FALSE
)
TABLESPACE pg_default;

ALTER TABLE public.statistics
  OWNER to collector;

CREATE TABLE public.notifications
(
  user_key text COLLATE pg_catalog."C" NOT NULL,
  last_notified timestamp without time zone NOT NULL,
  CONSTRAINT notifications_pkey PRIMARY KEY (user_key)
)
WITH (
  OIDS = FALSE
)
TABLESPACE pg_default;

ALTER TABLE public.notifications
  OWNER to collector;
