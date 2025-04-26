//declaration file for next grant precalculator 
#include <systemc.h>
const int CHANNELS = 4;
enum st_type {RESET, NEXT_GRANT};

SC_MODULE(NGPRC) {
    sc_in<bool> clk, reset;
    sc_in<sc_uint<CHANNELS> > request, grant;
    sc_out<sc_uint<CHANNELS> > nextGrant;
    sc_signal<sc_uint<CHANNELS> > priorityMask;
    sc_signal<st_type> state;
    sc_signal<sc_uint<CHANNELS> > concat_grant;
    
  void prc_preCalStateTransition();
  void prc_preCalOutputLogic();
    SC_CTOR(NGPRC) {
        SC_METHOD(prc_preCalStateTransition);
        sensitive << clk.pos() << reset;

        SC_METHOD(prc_preCalOutputLogic);
        sensitive << clk.pos() << reset;
    }
};
