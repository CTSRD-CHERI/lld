
// REQUIRES: clang

// Check x86_64 since MIPS n64 doesn't seem to use .rel.plt:
// RUN: %clang_cc1 -triple=x86_64-unknown-freebsd -emit-obj -O2 %s -o %t-amd64.o
// RUN: llvm-readobj -r %t-amd64.o | FileCheck -check-prefix AMD64-OBJ %s
// AMD64-OBJ: Relocations [
// AMD64-OBJ-NEXT: Section ({{.+}}) .rela.text {
// AMD64-OBJ-NEXT:     0x1 R_X86_64_PLT32 extern_function 0xFFFFFFFFFFFFFFFC
// AMD64-OBJ-NEXT:  }
// RUN: ld.lld -shared -o %t-amd64.so %t-amd64.o
// RUN: llvm-readobj -dynamic-table -r -sections -file-headers %t-amd64.so | FileCheck -check-prefix AMD64-SHLIB %s

// AMD64-SHLIB:      SectionHeaderCount: 14
// AMD64-SHLIB:  Section {
// AMD64-SHLIB:      Index: [[DYNSYM_INDEX:1]]
// AMD64-SHLIB-NEXT: Name: .dynsym

// AMD64-SHLIB:      Section {
// AMD64-SHLIB:         Index: 5
// AMD64-SHLIB:         Name: .rela.plt
// AMD64-SHLIB-NEXT:    Type: SHT_RELA (0x4)
// AMD64-SHLIB-NEXT:    Flags [ (0x2)
// AMD64-SHLIB-NEXT:      SHF_ALLOC (0x2)
// AMD64-SHLIB-NEXT:    ]
// AMD64-SHLIB-NEXT:    Address: [[PLT_REL_ADDR:.+]]
// AMD64-SHLIB-NEXT:    Offset: [[PLT_REL_ADDR]]
// AMD64-SHLIB-NEXT:    Size: 24
// This links to .dynsym
// AMD64-SHLIB-NEXT:    Link: [[DYNSYM_INDEX]]
// AMD64-SHLIB-NEXT:    Info: [[GOTPLT_INDEX:8]]
// AMD64-SHLIB-NEXT:    AddressAlignment: 8
// AMD64-SHLIB-NEXT:    EntrySize: 24
// AMD64-SHLIB-NEXT:  }

// AMD64-SHLIB:  Section {
// AMD64-SHLIB:      Index: [[GOTPLT_INDEX]]
// AMD64-SHLIB-NEXT: Name: .got.plt

// AMD64-SHLIB-LABEL: Relocations [
// AMD64-SHLIB-NEXT: Section ({{.+}}) .rela.plt {
// AMD64-SHLIB-NEXT:    0x2018 R_X86_64_JUMP_SLOT extern_function 0x0
// AMD64-SHLIB-NEXT:  }
// AMD64-SHLIB-LABEL: DynamicSection [
// AMD64-SHLIB-NEXT: Tag                Type                 Name/Value
// AMD64-SHLIB-NEXT: 0x0000000000000017 JMPREL               [[PLT_REL_ADDR]]
// AMD64-SHLIB-NEXT: 0x0000000000000002 PLTRELSZ             24 (bytes)
// AMD64-SHLIB-NEXT: 0x0000000000000003 PLTGOT               0x2000
// AMD64-SHLIB-NEXT: 0x0000000000000014 PLTREL               RELA

// RUN: llvm-strip -o /dev/stdout %t-amd64.so | llvm-readobj -file-headers - | FileCheck %s -check-prefix AMD64-STRIPPED
// AMD64-STRIPPED: SectionHeaderCount: 11


// Check that purecap also has the same link value and can be stripped

// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=plt -emit-obj -O2 %s -o %t.o
// RUN: llvm-readobj -r %t.o | FileCheck -check-prefix PURECAP-OBJ %s
// PURECAP-OBJ: Relocations [
// PURECAP-OBJ-NEXT: Section ({{.+}}) .rela.text {
// PURECAP-OBJ-NEXT:    0x8 R_MIPS_CHERI_CAPCALL20/R_MIPS_NONE/R_MIPS_NONE extern_function 0x0
// PURECAP-OBJ-NEXT:  }
// RUN: ld.lld -shared -o %t.so %t.o
// RUN: llvm-readobj -dynamic-table -r -file-headers -sections %t.so
// RUN: llvm-readobj -dynamic-table -file-headers -r -sections %t.so | FileCheck -check-prefix PURECAP-SHLIB %s
// PURECAP-SHLIB:      SectionHeaderCount: 18
// PURECAP-SHLIB:  Section {
// PURECAP-SHLIB:      Index: [[DYNSYM_INDEX:3]]
// PURECAP-SHLIB-NEXT: Name: .dynsym

// PURECAP-SHLIB:      Section {
// PURECAP-SHLIB:         Index: 7
// PURECAP-SHLIB:         Name: .rel.plt
// PURECAP-SHLIB-NEXT:    Type: SHT_REL (0x9)
// PURECAP-SHLIB-NEXT:    Flags [ (0x2)
// PURECAP-SHLIB-NEXT:      SHF_ALLOC (0x2)
// PURECAP-SHLIB-NEXT:    ]
// PURECAP-SHLIB-NEXT:    Address: [[PLT_REL_ADDR:.+]]
// PURECAP-SHLIB-NEXT:    Offset: [[PLT_REL_ADDR]]
// PURECAP-SHLIB-NEXT:    Size: 16
// PURECAP-SHLIB-NEXT:    Link: [[DYNSYM_INDEX]]
// PURECAP-SHLIB-NEXT:    Info: [[CAPTABLE_INDEX:11]]
// PURECAP-SHLIB-NEXT:    AddressAlignment: 8
// PURECAP-SHLIB-NEXT:    EntrySize: 16
// PURECAP-SHLIB-NEXT:  }

// PURECAP-SHLIB:  Section {
// PURECAP-SHLIB:      Index: [[CAPTABLE_INDEX]]
// PURECAP-SHLIB-NEXT: Name: .captable

// PURECAP-SHLIB: Relocations [
// PURECAP-SHLIB-NEXT: Section ({{.+}}) .rel.plt {
// PURECAP-SHLIB-NEXT:    0x30000 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_function 0x0 (real addend unknown)
// PURECAP-SHLIB-NEXT:  }
// PURECAP-SHLIB-LABEL: DynamicSection [
// PURECAP-SHLIB-NEXT: Tag                Type                 Name/Value
// PURECAP-SHLIB-NEXT: 0x0000000000000017 JMPREL               [[PLT_REL_ADDR:.+]]
// PURECAP-SHLIB-NEXT: 0x0000000000000002 PLTRELSZ             16 (bytes)
// PURECAP-SHLIB-NEXT: 0x0000000070000032 MIPS_PLTGOT          0x0
// PURECAP-SHLIB-NEXT: 0x0000000000000014 PLTREL               REL

// RUN: llvm-strip -o /dev/stdout %t.so | llvm-readobj -file-headers - | FileCheck %s -check-prefix PURECAP-STRIPPED
// PURECAP-STRIPPED: SectionHeaderCount: 14


extern void* extern_function(void);

void* call() {
  return extern_function();
}
