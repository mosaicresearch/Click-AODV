// -*- c-basic-offset: 4 -*-
#ifndef CLICK_BIGHASHMAPTEST_HH
#define CLICK_BIGHASHMAPTEST_HH
#include <click/element.hh>
CLICK_DECLS

/*
=c

BigHashMapTest()

=s test

runs regression tests for BigHashMap

=d

BigHashMapTest runs BigHashMap regression tests at initialization time. It
does not route packets.

*/

class BigHashMapTest : public Element { public:

    BigHashMapTest();
    ~BigHashMapTest();

    const char *class_name() const		{ return "BigHashMapTest"; }

    int initialize(ErrorHandler *);

};

CLICK_ENDDECLS
#endif
