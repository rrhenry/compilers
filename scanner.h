// scanner.c header file 
// Alexi Turcotte
// Roxanne Henry

#ifndef SCANNER_H
#define SCANNER_H

void qualident();
void type( int* ttp);
void FormParams();
void ImportList();
void StrucType();
void DeclSeq( int* displ);
void Module();
void ProcDecl();
void StatSeq( int displ);
void expr( int* ttp);
void stat();
void designator();
void ActParams();
void CaseStat();
void WhileStat();
void factor( int* ttp);
void term( int* ttp);
void identList();
void error( int e);

#endif