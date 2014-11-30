/* $Header: /home/robin/repository/netflow/flowtools.c,v 1.9 2002/05/21 21:53:02 robin Exp $ */

#include <Python.h>
#include <structmember.h>
#include <fcntl.h>
#include <stddef.h>
#include <arpa/inet.h>

#define HAVE_STRSEP 1
#include <ftlib.h>

#ifndef Py_TYPE
  #define Py_TYPE(ob) (((PyObject*)(ob))->ob_type)
#endif

typedef struct {
  PyObject_HEAD

  int fd;

  struct ftio io;
  int ftio_init_complete;

  struct fts3rec_offsets offsets;
  uint64_t xfield;
  int nflows;

} FlowSetObject;

typedef struct {
  PyObject_HEAD
  char *record;
  struct fts3rec_offsets offsets;
  uint64_t xfield;
  PyObject *parent;
} FlowObject;

typedef struct {
  PyObject_HEAD

  struct ftpdu ftpdu;
  struct fts3rec_offsets offsets;
  uint64_t xfield;
  uint32_t sequence;
  uint32_t sysUpTime, unix_secs, unix_nsecs;
  int length, count, version;
} FlowPDUObject;

typedef struct {
  PyObject_HEAD

  FlowPDUObject * pdu;
  int pos, offset;
} FlowPDUIterObject;

PyTypeObject FlowPDUType;
/* Define flow attributes */

enum RecordAttrType {
  RF_ADDR, RF_UINT32, RF_UINT16, RF_UINT8, RF_TIME
};

struct RecordAttrDef {
  enum RecordAttrType type;
  uint64_t xfield;
  int offset;
};

static PyObject * FlowObjectGetter(FlowObject * self, struct RecordAttrDef * f);

#define offset( x ) offsetof( struct fts3rec_offsets, x )
#define A(n, t, x) { #n, (getter) FlowObjectGetter, NULL, NULL, &(struct RecordAttrDef){ t, x, offset(n) } },
#define B(n, t, x, z) { #n, (getter) FlowObjectGetter, NULL, NULL, &(struct RecordAttrDef){ t, x, offset(z) } },

PyGetSetDef FlowObjectGS[] = {
  A(dFlows, RF_UINT32, FT_XFIELD_DFLOWS)
  A(dOctets, RF_UINT32, FT_XFIELD_DOCTETS)
  A(dPkts, RF_UINT32, FT_XFIELD_DPKTS)
  A(dst_as, RF_UINT16, FT_XFIELD_DST_AS)
  A(dst_mask, RF_UINT8, FT_XFIELD_DST_MASK)
  A(dst_tag, RF_UINT32, FT_XFIELD_DST_TAG)
  A(dstaddr, RF_ADDR, FT_XFIELD_DSTADDR)
  B(dstaddr_raw, RF_UINT32, FT_XFIELD_DSTADDR, dstaddr)
  A(dstport, RF_UINT16, FT_XFIELD_DSTPORT)
  A(engine_id, RF_UINT8, FT_XFIELD_ENGINE_ID)
  A(engine_type, RF_UINT8, FT_XFIELD_ENGINE_TYPE)
  A(exaddr, RF_ADDR, FT_XFIELD_EXADDR)
  B(exaddr_raw, RF_UINT32, FT_XFIELD_EXADDR, exaddr) 
  A(extra_pkts, RF_UINT32, FT_XFIELD_EXTRA_PKTS)
  B(first, RF_TIME, FT_XFIELD_FIRST, First)
  B(first_raw, RF_UINT32, FT_XFIELD_FIRST, First)
  A(in_encaps, RF_UINT8, FT_XFIELD_IN_ENCAPS)
  A(input, RF_UINT16, FT_XFIELD_INPUT)
  B(last, RF_TIME, FT_XFIELD_LAST, Last)
  B(last_raw, RF_UINT32, FT_XFIELD_LAST, Last)
  A(marked_tos, RF_UINT8, FT_XFIELD_MARKED_TOS)
  A(nexthop, RF_ADDR, FT_XFIELD_NEXTHOP)
  B(nexthop_raw, RF_UINT32, FT_XFIELD_NEXTHOP, nexthop)
  A(out_encaps, RF_UINT8, FT_XFIELD_OUT_ENCAPS)
  A(output, RF_UINT16, FT_XFIELD_OUTPUT)
  A(peer_nexthop, RF_ADDR, FT_XFIELD_PEER_NEXTHOP)
  B(peer_nexthop_raw, RF_UINT32, FT_XFIELD_PEER_NEXTHOP, peer_nexthop)
  A(prot, RF_UINT8, FT_XFIELD_PROT)
  A(router_sc, RF_UINT32, FT_XFIELD_ROUTER_SC)
  A(src_as, RF_UINT16, FT_XFIELD_SRC_AS)
  A(src_mask, RF_UINT8, FT_XFIELD_SRC_MASK)
  A(src_tag, RF_UINT32, FT_XFIELD_SRC_TAG)
  A(srcaddr, RF_ADDR, FT_XFIELD_SRCADDR)
  B(srcaddr_raw, RF_UINT32, FT_XFIELD_SRCADDR, srcaddr)
  A(srcport, RF_UINT16, FT_XFIELD_SRCPORT)
  A(sysUpTime, RF_UINT32, FT_XFIELD_SYSUPTIME)
  A(tcp_flags, RF_UINT8, FT_XFIELD_TCP_FLAGS)
  A(tos, RF_UINT8, FT_XFIELD_TOS)
  A(unix_nsecs, RF_UINT32, FT_XFIELD_UNIX_NSECS)
  A(unix_secs, RF_UINT32, FT_XFIELD_UNIX_SECS)
  { NULL }
};

#undef A
#undef B
#undef offset

// End define flow attributes

static PyObject *FlowToolsError;
static PyObject *FlowToolsIOError;
static PyObject *FlowToolsAttributeError;

static void FlowSetObjectDelete( FlowSetObject *self );
static PyObject *FlowSetObjectIter( FlowSetObject *o );
static PyObject *FlowSetObjectIterNext( FlowSetObject *o );
static int FlowSet_init(FlowSetObject *self, PyObject *args, PyObject* kwds);
static PyObject *FlowSetObject_write(FlowSetObject * self, PyObject * args, PyObject * kwds);

static struct PyMethodDef FlowSetMethods[] = {
    { "write", (PyCFunction)FlowSetObject_write, METH_VARARGS, "write!" },
    { NULL, NULL}
};


PyTypeObject FlowSetType = {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
        "flowtools.FlowSet",                        /* tp_name */
        sizeof( FlowSetObject),                     /* tp_basicsize */
        0,                                          /* tp_itemsize */
        (destructor)FlowSetObjectDelete,            /* tp_dealloc */
        0,                                          /* tp_print */
        0,                                          /* tp_getattr */
        0,                                          /* tp_setattr */
        0,                                          /* tp_compare */
        (reprfunc)0,                                /* tp_repr */
        0,                                          /* tp_as_number */
        0,                                          /* tp_as_sequence */
        0,                                          /* tp_as_mapping */
        (hashfunc)0,                                /* tp_hash */
        (ternaryfunc)0,                             /* tp_call */
        0,                                          /* tp_str */
        (getattrofunc)0,                            /* tp_getattro */
        (setattrofunc)0,                            /* tp_setattro */
        0,                                          /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_ITER,  /* tp_flags */
        "Stream of netflow data",                   /* tp_doc */
        (traverseproc)0,                            /* tp_traverse */
        (inquiry)0,                                 /* tp_clear */
        0,                                          /* tp_richcompare */
        0,                                          /* tp_weaklistoffset */
        (getiterfunc)FlowSetObjectIter,             /* tp_iter */
        (iternextfunc)FlowSetObjectIterNext,        /* tp_iternext */
        FlowSetMethods,                             /* tp_methods */
        0,                                          /* tp_members */
        0,                                          /* tp_getset */
        0,                                          /* tp_base */
        0,                                          /* tp_dict */
        0,                                          /* tp_descr_get */
        0,                                          /* tp_descr_set */
        0,                                          /* tp_dictoffset */
        (initproc) FlowSet_init,                    /* tp_init */
};

static void FlowObjectDelete( FlowObject *self );
static PyObject *FlowObjectGetID( FlowObject *self, PyObject* args );

static struct PyMethodDef FlowMethods[] = {
    { "getID", (PyCFunction)FlowObjectGetID, METH_VARARGS, "Return flow ID" },
    { NULL, NULL}	
};

PyTypeObject FlowType = {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
        "flowtools.Flow",                       /* tp_name */
        sizeof( FlowObject),                    /* tp_basicsize */
        0,                                      /* tp_itemsize */
        (destructor)FlowObjectDelete,           /* tp_dealloc */
        0,                                      /* tp_print */
        0,                                      /* tp_getattr */
        0,                                      /* tp_setattr */
        0,                                      /* tp_compare */
        (reprfunc)0,                            /* tp_repr */
        0,                                      /* tp_as_number */
        0,                                      /* tp_as_sequence */
        0,                                      /* tp_as_mapping */
        (hashfunc)0,                            /* tp_hash */
        (ternaryfunc)0,                         /* tp_call */
        0,                                      /* tp_str */
        (getattrofunc)0,                        /* tp_getattro*/
        (setattrofunc)0,                        /* tp_setattro */
        0,                                      /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,                     /* tp_flags */
        "Flow objects",                         /* tp_doc */
        (traverseproc)0,                        /* tp_traverse */
        (inquiry)0,                             /* tp_clear */
        0,                                      /* tp_richcompare */
        0,                                      /* tp_weaklistoffset */
        0,                                      /* tp_iter*/
        0,                                      /* tp_iternext */
        FlowMethods,                            /* tp_methods */
        0,                                      /* tp_members */
        FlowObjectGS,                           /* tp_getset */
        0,                                      /* tp_base */
        0,                                      /* tp_dict */
        0,                                      /* tp_descr_get */
        0,                                      /* tp_descr_set */
        0,                                      /* tp_dictoffset */
        0,                                      /* tp_init */
        0,                                      /* tp_alloc */
        0,                                      /* tp_new */
        0,                                      /* tp_free */
        0,                                      /* tp_is_gc */
};

static int FlowSet_init(FlowSetObject *self, PyObject *args, PyObject *kwds) {

    static char * kwlist[] = {
        "filename", "for_writing", NULL
    };

    char* file = NULL;
    PyObject * for_writing = NULL;

    struct ftver version = { 0 };
    int res = 0;
    int bForWriting = 0;
    int f_mmap = 0;

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "|sO", kwlist, &file, &for_writing) )
        return -1; 

    if (for_writing && PyBool_Check(for_writing) && (for_writing == Py_True))
      bForWriting = 1;

    if( file && strcmp( file , "-" ) != 0 ){
        f_mmap = FT_IO_FLAG_MMAP;
        Py_BEGIN_ALLOW_THREADS
        self->fd = open( file, bForWriting ? (O_CREAT | O_WRONLY) : O_RDONLY, 0644 );
        Py_END_ALLOW_THREADS

        if( self->fd < 0 ){
            PyErr_SetFromErrnoWithFilename( FlowToolsIOError, file );
            return -1;
        }
    }

    Py_BEGIN_ALLOW_THREADS
    res = ftio_init( &self->io, self->fd, bForWriting ? (FT_IO_FLAG_WRITE | FT_IO_FLAG_ZINIT | FT_IO_FLAG_NO_SWAP) : 
      (FT_IO_FLAG_READ | f_mmap));
    Py_END_ALLOW_THREADS

    if( res ) {
        PyErr_SetString( FlowToolsIOError, "ftio_init() failed" );
        return -1;
    }

    self->ftio_init_complete = 1;

    Py_BEGIN_ALLOW_THREADS

    if (bForWriting) {
      bzero(&version, sizeof(version));
      version.d_version = 5;
      version.s_version = FT_IO_SVERSION;
      ftio_set_ver( &self->io, &version);
      ftio_set_z_level(&self->io, 9);
      ftio_set_byte_order(&self->io, FT_HEADER_LITTLE_ENDIAN);
      ftio_set_flows_count(&self->io, 0);
      ftio_write_header(&self->io);
    } else {
      ftio_get_ver( &self->io, &version );
      fts3rec_compute_offsets( &self->offsets, &version );

      self->xfield = ftio_xfield( &self->io );
    }

    Py_END_ALLOW_THREADS

    return 0;
}

static void FlowSetObjectDelete( FlowSetObject *self )
{
    if (self->ftio_init_complete) {
      if ((self->io.flags & FT_IO_FLAG_WRITE) != 0) {
        //ftio_set_cap_time(&ftio, cap_file.time, (u_int32)tt_now);
        //ftio_set_corrupt(&ftio, cap_file.hdr_flows_corrupt);
        //ftio_set_lost(&ftio, cap_file.hdr_flows_lost);
        //ftio_set_reset(&ftio, cap_file.hdr_flows_reset);
        ftio_set_flows_count(&self->io, self->nflows);

        /* re-write header first */
        Py_BEGIN_ALLOW_THREADS
        ftio_write_header(&self->io);
        Py_END_ALLOW_THREADS
      }

      Py_BEGIN_ALLOW_THREADS
      ftio_close( &(self->io) );
      Py_END_ALLOW_THREADS
    }

    if( self->fd ) {
      Py_BEGIN_ALLOW_THREADS
      close( self->fd );
      Py_END_ALLOW_THREADS
    }

    Py_TYPE(self)->tp_free(self);
}

static PyObject *FlowSetObjectIter( FlowSetObject *self )
{
    if ((self->io.flags & FT_IO_FLAG_READ) == 0) {
      PyErr_SetNone(PyExc_ValueError);
      return NULL;
    }

    Py_XINCREF(self);
    return (PyObject *)self;
}

static PyObject *FlowSetObjectIterNext( FlowSetObject *self )
{
    FlowObject *flow;
    char *record;
    
    if ((self->io.flags & FT_IO_FLAG_READ) == 0) {
      PyErr_SetNone(PyExc_ValueError);
      return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    
    record = ftio_read( &self->io );

    Py_END_ALLOW_THREADS
    
    if( ! record ){
        PyErr_SetNone( PyExc_StopIteration );
        return NULL;
    }
    
	flow = PyObject_NEW( FlowObject, &FlowType );
    if( ! flow ) return NULL;
    flow->record = record;
    flow->parent = (PyObject*) self;
    flow->xfield = self->xfield;
    memcpy(&flow->offsets, &self->offsets, sizeof(self->offsets));
    Py_XINCREF( self );
    
    return (PyObject *)flow;
}

static PyObject *FlowSetObject_write(FlowSetObject * self, PyObject * args, PyObject * kwds) {

  FlowPDUObject * PDU = NULL;

  int i, offset;
  int res = 0;

  if ((self->io.flags & FT_IO_FLAG_WRITE) == 0) {
    PyErr_SetNone(PyExc_ValueError);
    return NULL;
  }

  if( ! PyArg_ParseTuple( args, "O!", &FlowPDUType, &PDU ) ) return NULL;

  Py_XINCREF(PDU);
  Py_BEGIN_ALLOW_THREADS

  for (i = 0, offset = 0; i < PDU->ftpdu.ftd.count;
      ++i, offset += PDU->ftpdu.ftd.rec_size)
      if ((res = ftio_write(&self->io, (char*)PDU->ftpdu.ftd.buf+offset)) < 0)
        goto resout;

resout:
  Py_END_ALLOW_THREADS
  Py_XDECREF(PDU);

  self->nflows += i;

  if (res < 0) {
    PyErr_SetString( FlowToolsIOError, "Error writing the flow" );
    return NULL;
  }

  Py_XINCREF(Py_None);

  return Py_None;

}

static void FlowObjectDelete( FlowObject *self )
{
    Py_XDECREF( self->parent );
    Py_TYPE(self)->tp_free(self);
}

#define getoffset( f ) ( * ( (uint16_t *)( (void *)( &self->offsets ) + f->offset ) ) )

static PyObject * FlowObjectGetter(FlowObject * self, struct RecordAttrDef * f) {
  uint32_t addr;
  uint32_t unix_secs, unix_nsecs, sysUpTime;
  struct fttime time;

  if( ! ( self->xfield & f->xfield ) ){
    PyErr_SetString( FlowToolsAttributeError, "Attribute not supported by flow type" );
    return NULL;
  }

  switch (f->type) {
    case RF_ADDR:
      addr = ntohl( *( (uint32_t *)( self->record + getoffset( f ) ) ) );
      return Py_BuildValue( "s",  (char *) inet_ntoa( *(struct in_addr *)&addr ) );

    case RF_UINT8:
      return Py_BuildValue( "i", (int) *( (uint8_t *)( self->record + getoffset( f ) ) ) );

    case RF_UINT16:
      return Py_BuildValue( "i", (int) *( (uint16_t *)( self->record + getoffset( f ) ) ) );

    case RF_UINT32:
      return PyLong_FromUnsignedLong( (unsigned int)*( (uint32_t *)( self->record + getoffset( f ) ) ) );

    case RF_TIME:
      unix_secs = *( (uint32_t *)( self->record + self->offsets.unix_secs ) );
      unix_nsecs = *( (uint32_t *)( self->record + self->offsets.unix_nsecs ) );
      sysUpTime = *( (uint32_t *)( self->record + self->offsets.sysUpTime ) );
      time = ftltime( sysUpTime, unix_secs, unix_nsecs,
        *( (uint32_t *)( self->record + getoffset( f ) ) ) );
      return Py_BuildValue( "f", time.secs + time.msecs * 1e-3 );
  }

  return NULL;
}

static PyObject *FlowObjectGetID( FlowObject *self, PyObject *args )
{
    char buffer[18];
    char src[8];
    char dst[8];
    int bidir = 0;
    char *p;
    
    if( ! PyArg_ParseTuple( args, "|i", &bidir ) ) return NULL;

    p = src;
    memcpy( p, self->record + self->offsets.srcaddr, sizeof( uint32_t ) );
    p += sizeof( uint32_t );
    memcpy( p, self->record + self->offsets.srcport, sizeof( uint16_t ) );
    p += sizeof( uint16_t );
    memcpy( p, self->record + self->offsets.input, sizeof( uint16_t ) );
    
    p = dst;
    memcpy( p, self->record + self->offsets.dstaddr, sizeof( uint32_t ) );
    p += sizeof( uint32_t );
    memcpy( p, self->record + self->offsets.dstport, sizeof( uint16_t ) );
    p += sizeof( uint16_t );
    memcpy( p, self->record + self->offsets.output, sizeof( uint16_t ) );
    
    p = buffer;
    if( ( ! bidir ) || ( memcmp( src, dst, sizeof( src ) ) < 0 ) ){
        memcpy( p, src, sizeof( src ) );
        p += sizeof( src );
        memcpy( p, dst, sizeof( dst ) );
        p += sizeof( dst );
    }
    else{
        memcpy( p, dst, sizeof( dst ) );
        p += sizeof( dst ); 
        memcpy( p, src, sizeof( src ) );
        p += sizeof( src );
    }

    memcpy( p, self->record + self->offsets.prot, sizeof( uint8_t ) );
        
    return Py_BuildValue( "s#", buffer, sizeof( buffer ) );
}

static PyObject *FlowPDUIter_Iter( FlowPDUIterObject *self )
{
    Py_XINCREF(self);
    return (PyObject *)self;
}

static PyObject *FlowPDUIter_Next( FlowPDUIterObject *self )
{
    FlowObject * flow;

    if (self->pos >= self->pdu->ftpdu.ftd.count) {
        PyErr_SetNone( PyExc_StopIteration );
        return NULL;
    }

    flow = PyObject_NEW( FlowObject, &FlowType );

    if( ! flow ) return NULL;

    flow->record = (char*) (self->pdu->ftpdu.ftd.buf + self->offset);
    flow->parent = (PyObject *) self->pdu;
    flow->xfield = self->pdu->xfield;
    memcpy(&flow->offsets, &self->pdu->offsets, sizeof(self->pdu->offsets));

    self->pos++;
    self->offset += self->pdu->ftpdu.ftd.rec_size;

    Py_XINCREF( self->pdu );
    return (PyObject *)flow;
}

static void FlowPDUIter_Delete( FlowPDUIterObject *self )
{
    Py_XDECREF( self->pdu );
    Py_TYPE(self)->tp_free(self);
}


PyTypeObject FlowPDUIterType = {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
        "flowtools.FlowPDUIter",                    /* tp_name */
        sizeof( FlowPDUIterObject),                 /* tp_basicsize */
        0,                                          /* tp_itemsize */
        (destructor) FlowPDUIter_Delete,            /* tp_dealloc */
        0,                                          /* tp_print */
        0,                                          /* tp_getattr */
        0,                                          /* tp_setattr */
        0,                                          /* tp_compare */
        (reprfunc)0,                                /* tp_repr */
        0,                                          /* tp_as_number */
        0,                                          /* tp_as_sequence */
        0,                                          /* tp_as_mapping */
        (hashfunc)0,                                /* tp_hash */
        (ternaryfunc)0,                             /* tp_call */
        0,                                          /* tp_str */
        (getattrofunc)0,                            /* tp_getattro */
        (setattrofunc)0,                            /* tp_setattro */
        0,                                          /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_ITER,  /* tp_flags */
        "Iterator for PDU",                         /* tp_doc */
        (traverseproc)0,                            /* tp_traverse */
        (inquiry)0,                                 /* tp_clear */
        0,                                          /* tp_richcompare */
        0,                                          /* tp_weaklistoffset */
        (getiterfunc) FlowPDUIter_Iter,             /* tp_iter */
        (iternextfunc) FlowPDUIter_Next,            /* tp_iternext */
        0,                                          /* tp_methods */
        0,                                          /* tp_members */
        0,                                          /* tp_getset */
        0,                                          /* tp_base */
        0,                                          /* tp_dict */
        0,                                          /* tp_descr_get */
        0,                                          /* tp_descr_set */
        0,                                          /* tp_dictoffset */
        0,                                          /* tp_init */
};


static int FlowPDU_init(FlowPDUObject * self, PyObject * args, PyObject * kwds) {

  static char * kwlist[] = {
    "exporter", "buffer", NULL
  };

  char * buf;
  int buflen;
  uint32_t exporter_ip;

  int res = 0;
  struct ftpdu_header * ph = NULL;

  if( ! PyArg_ParseTupleAndKeywords( args, kwds, "Is#", kwlist, &exporter_ip, &buf, &buflen ) ) 
    return -1;

  bzero (&self->ftpdu, sizeof(self->ftpdu));
  memcpy(self->ftpdu.buf, buf, buflen);

  Py_BEGIN_ALLOW_THREADS

  self->ftpdu.ftd.exporter_ip = exporter_ip;
  self->ftpdu.ftd.byte_order = FT_HEADER_LITTLE_ENDIAN;
  self->ftpdu.bused = buflen;

  if ((res = ftpdu_verify(&self->ftpdu)) < 0)
    goto resout;

  ph = (struct ftpdu_header *) &self->ftpdu.buf;

  self->version = ph->version;

  self->sequence = ph->flow_sequence;
  self->count = ph->count;
  
  self->sysUpTime = ph->sysUpTime;
  self->unix_secs = ph->unix_secs;
  self->unix_nsecs = ph->unix_nsecs;

#if BYTE_ORDER == LITTLE_ENDIAN
  SWAPINT32(self->sequence);
  SWAPINT16(self->count);
  SWAPINT16(self->version);

  SWAPINT32(self->sysUpTime);
  SWAPINT32(self->unix_secs);
  SWAPINT32(self->unix_nsecs);
#endif

  self->length = fts3rec_pdu_decode(&self->ftpdu);

  self->xfield = ftrec_xfield(&self->ftpdu.ftv);
  fts3rec_compute_offsets( &self->offsets, &self->ftpdu.ftv );

resout:
  Py_END_ALLOW_THREADS

  if (res < 0) 
    return -1;

  return 0;
}

static PyObject *FlowPDU_Iter( FlowPDUObject *self )
{
    FlowPDUIterObject * iter = PyObject_NEW(FlowPDUIterObject, &FlowPDUIterType);

    iter->pdu = self;
    iter->pos = iter->offset = 0;

    Py_XINCREF(self);
    return (PyObject *) iter;
}


static PyObject* Py_ReturnBool(const int x) {
  if (x) Py_RETURN_TRUE;
  else Py_RETURN_FALSE;
}

static int FlowPDU_IsAdjacent_Helper(FlowPDUObject * o1, FlowPDUObject * o2) {
  return (((o1->sequence + o1->count) % (uint32_t) 0xffffffff) == o2->sequence) &&
    (o1->sysUpTime <= o2->sysUpTime) && (o1->unix_secs <= o2->unix_secs) &&
    (o2->unix_nsecs <= o2->unix_nsecs);
}

static PyObject* FlowPDU_Compare_Helper(FlowPDUObject * o1, FlowPDUObject * o2) {
  if (FlowPDU_IsAdjacent_Helper(o1, o2))
    Py_RETURN_TRUE;

  if (o1->sequence == o2->sequence) {
    if (o1->unix_secs < o2->unix_secs)
      Py_RETURN_TRUE;
    if ((o1->unix_secs == o2->unix_secs) && (o1->unix_nsecs == o2->unix_nsecs))
      Py_RETURN_TRUE;
  } else if (o1->sequence < o2->sequence) {
    if (o1->sysUpTime <= o2->sysUpTime) {
      if (o1->unix_secs < o2->unix_secs)
        Py_RETURN_TRUE;
      else 
        if ((o1->unix_secs == o2->unix_secs) && (o1->unix_nsecs <= o2->unix_nsecs))
          Py_RETURN_TRUE;
    } else {
      if (o1->unix_secs > o2->unix_secs)
        Py_RETURN_TRUE;
      else
        if ((o1->unix_secs == o2->unix_secs) && (o1->unix_nsecs >= o2->unix_nsecs))
          Py_RETURN_TRUE;
    }
  } 
          
  Py_RETURN_FALSE;
}

static PyObject* FlowPDU_RichCompare(FlowPDUObject * o1, FlowPDUObject * o2, int opid) {
  if ((PyObject_IsInstance((PyObject *) o1, (PyObject *) &FlowPDUType) != 1) ||
    (PyObject_IsInstance((PyObject *) o1, (PyObject *) &FlowPDUType) != 1)) {
    if (PyErr_Occurred() == NULL)
      PyErr_SetString(PyExc_TypeError, "Can only compare to FlowPDU");
    return NULL;
  }

  if (o1->ftpdu.bused != o2->ftpdu.bused) {
    if (opid == Py_NE) Py_RETURN_TRUE;
    if (opid == Py_EQ) Py_RETURN_FALSE;
  }

  if ((opid == Py_NE) || (opid == Py_EQ) || (opid == Py_LE) || (opid == Py_GE)) {
    if (memcmp(o1->ftpdu.buf, o2->ftpdu.buf, o1->ftpdu.bused) == 0)
      return Py_ReturnBool((opid == Py_EQ) || (opid == Py_LE) || (opid == Py_GE));
    else 
      if ((opid == Py_NE) || (opid == Py_EQ))
        return Py_ReturnBool(opid == Py_NE);
  }

  if (o1->ftpdu.ftd.exporter_ip != o2->ftpdu.ftd.exporter_ip) {
    Py_RETURN_FALSE;
  }

  if ((opid == Py_LT) || (opid == Py_LE))
    return FlowPDU_Compare_Helper(o1, o2);
  else
    return FlowPDU_Compare_Helper(o2, o1);
}

static PyObject *FlowPDU_IsNext(FlowPDUObject * self, PyObject * args, PyObject * kwds) {

  FlowPDUObject * PDU = NULL;

  if( ! PyArg_ParseTuple( args, "O!", &FlowPDUType, &PDU ) ) return NULL;

  return Py_ReturnBool(FlowPDU_IsAdjacent_Helper(self, PDU));
}

static void FlowPDU_Delete( FlowPDUObject *self )
{
    Py_TYPE(self)->tp_free(self);
}

static struct PyMemberDef FlowPDU_Members[] = {
  { "version", T_INT, offsetof(FlowPDUObject, version), READONLY,
    "unsigned int -> Netflow version." },
  { "sequence", T_UINT, offsetof(FlowPDUObject, sequence), READONLY,
    "unsigned int -> Seq counter of total flows seen." },
  { "count", T_INT, offsetof(FlowPDUObject, count), READONLY,
    "unsigned int -> The number of records in the PDU." },
  { "sysUpTime", T_UINT, offsetof(FlowPDUObject, sysUpTime), READONLY,
    "unsigned int -> Current time in millisecs since router booted." },
  { "unix_secs", T_UINT, offsetof(FlowPDUObject, unix_secs), READONLY,
    "unsigned int -> Current seconds since 0000 UTC 1970." },
  { "unix_nsecs", T_UINT, offsetof(FlowPDUObject, unix_nsecs), READONLY,
    "unsigned int -> Residual nanoseconds since 0000 UTC 1970." },
  { 0 } };

static struct PyMethodDef FlowPDU_Methods[] = {
  { "is_next", (PyCFunction)FlowPDU_IsNext, METH_VARARGS, 
    "Check if given flow is next to self.\n\n"
    "Return true if PDU goes immediately after self" },
  { NULL, NULL}
};


PyTypeObject FlowPDUType = {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
        "flowtools.FlowPDU",                        /* tp_name */
        sizeof( FlowPDUObject),                     /* tp_basicsize */
        0,                                          /* tp_itemsize */
        (destructor) FlowPDU_Delete,                /* tp_dealloc */
        0,                                          /* tp_print */
        0,                                          /* tp_getattr */
        0,                                          /* tp_setattr */
        0,                                          /* tp_compare */
        (reprfunc)0,                                /* tp_repr */
        0,                                          /* tp_as_number */
        0,                                          /* tp_as_sequence */
        0,                                          /* tp_as_mapping */
        (hashfunc)0,                                /* tp_hash */
        (ternaryfunc)0,                             /* tp_call */
        0,                                          /* tp_str */
        (getattrofunc)0,                            /* tp_getattro */
        (setattrofunc)0,                            /* tp_setattro */
        0,                                          /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_ITER,  /* tp_flags */
        "FlowPDU(int exporter, string buffer) -> FlowPDU object.\n\n"
        "Contains flow header attributes. Can be iterated over to get\n"
        "individual flows.",                        /* tp_doc */
        (traverseproc)0,                            /* tp_traverse */
        (inquiry)0,                                 /* tp_clear */
        (richcmpfunc) FlowPDU_RichCompare,          /* tp_richcompare */
        0,                                          /* tp_weaklistoffset */
        (getiterfunc)FlowPDU_Iter,                  /* tp_iter */
        0,                                          /* tp_iternext */
        FlowPDU_Methods,                            /* tp_methods */
        FlowPDU_Members,                            /* tp_members */
        0,                                          /* tp_getset */
        0,                                          /* tp_base */
        0,                                          /* tp_dict */
        0,                                          /* tp_descr_get */
        0,                                          /* tp_descr_set */
        0,                                          /* tp_dictoffset */
        (initproc) FlowPDU_init,                    /* tp_init */
};


static struct PyMethodDef FlowToolsMethods[] = {
    { NULL }
};

static char* FlowToolsModuleDescription = 
  "Python interface to OSU flow-tools library.\n\n"
  "This module allows you to read, parse, and write netflow PDUs";

static void InitExceptions(PyObject *module_dict) {
  PyObject *t;

  FlowToolsError = PyErr_NewException( "flowtools.Error", NULL, NULL );
  PyDict_SetItemString( module_dict, "Error", FlowToolsError );

  t = PyTuple_Pack(2, FlowToolsError, PyExc_IOError);
  FlowToolsIOError = PyErr_NewException( "flowtools.IOError", t, NULL );
  Py_XDECREF(t);
  PyDict_SetItemString( module_dict, "IOError", FlowToolsIOError );

  t = PyTuple_Pack(2, FlowToolsError, PyExc_AttributeError);
  FlowToolsAttributeError = PyErr_NewException( "flowtools.AttributeError", t, NULL );
  Py_XDECREF(t);
  PyDict_SetItemString( module_dict, "AttributeError", FlowToolsAttributeError );
}

static struct PyModuleDef moduledef = {
  PyModuleDef_HEAD_INIT,
  "flowtools",                                   /* m_name */
  "Python interface to OSU flow-tools library",  /* m_doc */
  -1,                                            /* m_size */
  FlowToolsMethods,                              /* m_methods */
  NULL,                                          /* m_reload */
  NULL,                                          /* m_traverse */
  NULL,                                          /* m_clear */
  NULL,                                          /* m_free */
};

PyMODINIT_FUNC PyInit_flowtools()
{
    PyObject *d, *m;

    FlowSetType.tp_new = PyType_GenericNew;
    FlowType.tp_new = PyType_GenericNew;
    FlowPDUType.tp_new = PyType_GenericNew;
    FlowPDUIterType.tp_new = PyType_GenericNew;

    if ((PyType_Ready(&FlowSetType) < 0) || 
        (PyType_Ready(&FlowType) < 0) ||
        (PyType_Ready(&FlowPDUType) < 0) ||
        (PyType_Ready(&FlowPDUIterType) < 0))
      return;

    m = PyModule_Create(&moduledef);
    if (m == NULL)
      return NULL;
    
    Py_INCREF(&FlowSetType);
    Py_INCREF(&FlowPDUType);
    Py_INCREF(&FlowType);

    PyModule_AddObject(m, "FlowSet", (PyObject *) &FlowSetType);
    PyModule_AddObject(m, "FlowPDU", (PyObject *) &FlowPDUType);
    PyModule_AddObject(m, "Flow", (PyObject *) &FlowType);

    d = PyModule_GetDict( m );
    InitExceptions(d);

    return m;
}

