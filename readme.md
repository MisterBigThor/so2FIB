
# MEMORIA DEL ZeOS


2. INTRODUCCION

3. ENTER THE SYSTEM

### PILA DE SISTEMA

### CAMBIOS EN LOS ARCHIVOS


#### WRITING THE WRAPPER
* [wrappers.S](wrappers.S) -> codigo en asm que nos proporciona un punto de entrada donde podemos guardar el contexto y hacer la operacion int 0x80 o sysenter. Los parametros se pasan de derecha a izquierda por %ebx, %ecx, %edx, %esi, %edi.

#### IDT INITIALIZATION
* [hardware.c](hardware.c) -> enable_int(void) hemos actualizado los bits para activar la interrupcion de reloj y de teclado.
* [interrupt.c](interrup.c) -> Se llama desde el main del SO para inicializar la IDT, añadimos una entrada en la IDT las interrupciones que necesitemos.
#### WRITING THE HANDLER
* []() -> El handler es donde llamaremos desde la IDT, se debe comprobar el numero de llamada a SO y se llama a la [sys_call_table](sys_call_table.S) donde estan definidas las llamadas a las sys_*.
#### WRITING THE SERVICE ROUTINE
* [sys.c](sys.c) -> Mecanismo de sistema al que llegaran desde la [sys_call_table.S](sys_call_table.S) y que implementa la operacion en si.
* [errno.h](include/errno.h) aqui definimos los codigos de error del sistema, que utilizamos en sys.c.

#### FAST SYSTEM CALLS


4. BASIC PROCESS MANAGEMENT

