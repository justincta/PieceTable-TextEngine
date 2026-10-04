# Text Editor - Piece Table Implementation

## 1. Descrierea Proiectului
Acest proiect implementeaza un motor de editare text de inalta performanta bazat pe structura de date Piece Table. Editorul este capabil sa gestioneze fisiere de mari dimensiuni (peste 8MB) cu un consum minim de memorie si operatii de inserare/stergere extrem de rapide, indiferent de pozitia cursorului.

## 2. Fisierele Proiectului
- **text_editor.h**: Header-ul care contine definitiile structurilor (PieceCell, PieceList, TextEditor) si prototipurile functiilor.
- **text_editor.c**: Implementarea logicii editorului, gestiunea memoriei si a listei dublu inlantuite.
- **Makefile**: Script de compilare automatizata.
- **README.md**: Documentatia proiectului.

## 3. Structura de Date: Piece Table
Spre deosebire de un editor clasic care stocheaza tot textul intr-un singur array (string), acest editor foloseste Piece Table:
- **Original Buffer**: Un buffer read-only care contine continutul initial al fisierului.
- **Add Buffer**: Un buffer append-only unde se stocheaza tot textul nou introdus de utilizator.
- **Piece List**: O lista dublu inlantuita de "piese". Fiecare piesa (PieceCell) contine:
    - Un pointer catre buffer-ul sursa (Original sau Add).
    - Indexul de start si lungimea secventei de text.

**De ce Lista Dublu Inlantuita?**
Am ales o lista dublu inlantuita pentru a permite parcurgerea eficienta in ambele sensuri si pentru a facilita operatia de Split (taierea unei piese la inserarea in mijloc) in timp constant odata ce piesa este localizata.

## 4. Complexitatea Operatiilor Implementate
Complexitatea este raportata la numarul de piese existente in lista (N_piese) si numarul total de caractere (N_caractere).

- **Inserare (add_text) - O(N_piese)**: Localizarea pozitiei cursorului necesita o parcurgere liniara a listei de piese. Inserarea propriu-zisa a noului nod sau fuziunea cu piesa anterioara se realizeaza in timp constant O(1).
- **Stergere (delete_text) - O(N_piese)**: Localizeaza punctul de start al stergerii si parcurge lista pentru a elimina sau redimensiona piesele afectate de operatie.
- **Afisare lungime (show_total_len) - O(1)**: Deoarece lungimea totala este stocata intr-o variabila in structura editorului care se actualizeaza la fiecare modificare, returnarea lungimii este instantanee.
- **Cautare Cursor - O(N_piese)**: Parcurge lista nod cu nod pana cand suma lungimilor pieselor vizitate atinge pozitia globala a cursorului.
- **Undo / Redo - O(N_piese)**: Aceasta operatie realizeaza o copie a structurii de piese (nodurile listei), fara a copia textul din buffere, fiind proportionala cu numarul de elemente din lista.
- **Extragere text (extract_current_text) - O(N_caractere)**: Reprezinta operatia care depinde de volumul total de date, deoarece necesita vizitarea tuturor pieselor si copierea efectiva a octetilor din buffere intr-un string nou, continuu.

## 5. Optimizari pentru Performanta (8MB+ Input)
Pentru a gestiona eficient milioane de caractere, am implementat urmatoarele strategii:
- **Piece Merging**: Daca utilizatorul scrie caractere consecutive, editorul nu creeaza piese noi, ci extinde lungimea ultimei piese daca aceasta provine din Add Buffer. Acest lucru previne fragmentarea listei.
- **Exponential Realloc**: Add Buffer isi dubleaza capacitatea la fiecare umplere (1MB -> 2MB -> 4MB), reducand numarul de apeluri costisitoare de sistem.
- **Cursor Caching**: Editorul retine ultima piesa accesata. La inserari secventiale, cautarea nu mai porneste de la inceputul listei, ci direct de la pozitia anterioara.
- **Efficient Undo/Redo**: Se realizeaza prin "Snapshots" ale listei de piese. Deoarece bufferele sunt append-only, Undo/Redo nu copiaza textul, ci doar structura usoara a listei.

## 6. Makefile
Proiectul include un Makefile cu urmatoarele reguli:
- **make**: Compileaza sursele si genereaza executabilul.
- **make clean**: Sterge fisierele obiect si executabilul.
- **make run**: Ruleaza editorul.


## 7. Alete note

- De aici as continua sa implementez si undo, redo folosind o stiva, in care inserez caracterele sterse, si pentru a o optimiza aplic si Piece Merging

- Am ales aceasta structura de date conform articolului urmator:
https://dev.to/_darrenburns/the-piece-table---the-unsung-hero-of-your-text-editor-al8
