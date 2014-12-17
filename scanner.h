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
void DeclSeq();
void Module();
void ProcDecl();
void StatSeq();
void expr( int ttp);
void stat();
void designator();
void ActParams();
void CaseStat();
void WhileStat();
void factor();
void term();

#endif