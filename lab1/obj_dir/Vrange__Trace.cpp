// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Tracing implementation internals
#include "verilated_vcd_c.h"
#include "Vrange__Syms.h"


//======================

void Vrange::traceChg(VerilatedVcd* vcdp, void* userthis, uint32_t code) {
    // Callback from vcd->dump()
    Vrange* t = (Vrange*)userthis;
    Vrange__Syms* __restrict vlSymsp = t->__VlSymsp;  // Setup global symbol table
    if (vlSymsp->getClearActivity()) {
        t->traceChgThis(vlSymsp, vcdp, code);
    }
}

//======================


void Vrange::traceChgThis(Vrange__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code) {
    Vrange* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    int c = code;
    if (0 && vcdp && c) {}  // Prevent unused
    // Body
    {
        if (VL_UNLIKELY((1U & (vlTOPp->__Vm_traceActivity 
                               | (vlTOPp->__Vm_traceActivity 
                                  >> 2U))))) {
            vlTOPp->traceChgThis__2(vlSymsp, vcdp, code);
        }
        if (VL_UNLIKELY((1U & (vlTOPp->__Vm_traceActivity 
                               | (vlTOPp->__Vm_traceActivity 
                                  >> 4U))))) {
            vlTOPp->traceChgThis__3(vlSymsp, vcdp, code);
        }
        if (VL_UNLIKELY((2U & vlTOPp->__Vm_traceActivity))) {
            vlTOPp->traceChgThis__4(vlSymsp, vcdp, code);
        }
        if (VL_UNLIKELY((4U & vlTOPp->__Vm_traceActivity))) {
            vlTOPp->traceChgThis__5(vlSymsp, vcdp, code);
        }
        if (VL_UNLIKELY((8U & vlTOPp->__Vm_traceActivity))) {
            vlTOPp->traceChgThis__6(vlSymsp, vcdp, code);
        }
        vlTOPp->traceChgThis__7(vlSymsp, vcdp, code);
    }
    // Final
    vlTOPp->__Vm_traceActivity = 0U;
}

void Vrange::traceChgThis__2(Vrange__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code) {
    Vrange* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    int c = code;
    if (0 && vcdp && c) {}  // Prevent unused
    // Body
    {
        vcdp->chgBit(c+1,(vlTOPp->range__DOT__running));
    }
}

void Vrange::traceChgThis__3(Vrange__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code) {
    Vrange* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    int c = code;
    if (0 && vcdp && c) {}  // Prevent unused
    // Body
    {
        vcdp->chgBus(c+9,(vlTOPp->range__DOT__addr),4);
    }
}

void Vrange::traceChgThis__4(Vrange__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code) {
    Vrange* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    int c = code;
    if (0 && vcdp && c) {}  // Prevent unused
    // Body
    {
        vcdp->chgBus(c+17,(vlTOPp->range__DOT__mem[0]),16);
        vcdp->chgBus(c+18,(vlTOPp->range__DOT__mem[1]),16);
        vcdp->chgBus(c+19,(vlTOPp->range__DOT__mem[2]),16);
        vcdp->chgBus(c+20,(vlTOPp->range__DOT__mem[3]),16);
        vcdp->chgBus(c+21,(vlTOPp->range__DOT__mem[4]),16);
        vcdp->chgBus(c+22,(vlTOPp->range__DOT__mem[5]),16);
        vcdp->chgBus(c+23,(vlTOPp->range__DOT__mem[6]),16);
        vcdp->chgBus(c+24,(vlTOPp->range__DOT__mem[7]),16);
        vcdp->chgBus(c+25,(vlTOPp->range__DOT__mem[8]),16);
        vcdp->chgBus(c+26,(vlTOPp->range__DOT__mem[9]),16);
        vcdp->chgBus(c+27,(vlTOPp->range__DOT__mem[10]),16);
        vcdp->chgBus(c+28,(vlTOPp->range__DOT__mem[11]),16);
        vcdp->chgBus(c+29,(vlTOPp->range__DOT__mem[12]),16);
        vcdp->chgBus(c+30,(vlTOPp->range__DOT__mem[13]),16);
        vcdp->chgBus(c+31,(vlTOPp->range__DOT__mem[14]),16);
        vcdp->chgBus(c+32,(vlTOPp->range__DOT__mem[15]),16);
        vcdp->chgBus(c+145,(vlTOPp->range__DOT__c1__DOT__dout),32);
    }
}

void Vrange::traceChgThis__5(Vrange__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code) {
    Vrange* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    int c = code;
    if (0 && vcdp && c) {}  // Prevent unused
    // Body
    {
        vcdp->chgBit(c+153,(vlTOPp->range__DOT__cgo));
        vcdp->chgBit(c+161,(vlTOPp->range__DOT__cdone));
        vcdp->chgBus(c+169,(vlTOPp->range__DOT__n),32);
        vcdp->chgBus(c+177,(vlTOPp->range__DOT__num),4);
        vcdp->chgBus(c+185,(vlTOPp->range__DOT__din),16);
    }
}

void Vrange::traceChgThis__6(Vrange__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code) {
    Vrange* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    int c = code;
    if (0 && vcdp && c) {}  // Prevent unused
    // Body
    {
        vcdp->chgBit(c+193,(vlTOPp->range__DOT__we));
    }
}

void Vrange::traceChgThis__7(Vrange__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code) {
    Vrange* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    int c = code;
    if (0 && vcdp && c) {}  // Prevent unused
    // Body
    {
        vcdp->chgBit(c+201,(vlTOPp->clk));
        vcdp->chgBit(c+209,(vlTOPp->go));
        vcdp->chgBus(c+217,(vlTOPp->start),32);
        vcdp->chgBit(c+225,(vlTOPp->done));
        vcdp->chgBus(c+233,(vlTOPp->count),16);
    }
}
