# Índex de documentació ONEGAI

Aquest directori barreja GDD, plans, auditories i guies tècniques. Aquesta pàgina serveix com a porta d'entrada per saber quin document obrir segons la feina.

## Lectura recomanada

| Necessitat | Document |
|---|---|
| Entendre el codi Spring/Thymeleaf i on tocar cada cosa | `Documentacio_Codi.md` |
| Entendre el sistema de cartes, tiers, vida i stats | `Sistema_Cartas_Tiers.md` |
| Revisar l'estat del projecte, objectius assolits i DAFO | `Estat_Projecte_i_DAFO.md` |
| Gestionar commits, branques i rutina de versions | `Guia_Control_Versions.md` |
| Entendre l'esquema de branques (public/development/qa/fixing/eines) | `Guia_Branques.md` |
| Aplicar la norma de llengua i noms de codi | `Convencions_Llengua_i_Codi.md` |
| Crear contingut amb prompts consistents | `Guia_Prompts_Mundo_y_Creacion.md` i `Plantilla_Prompt_Contenido.md` |
| Planificar habilitats per classe | `Plan_Habilidades_por_Clase.md` |
| Revisar mapa mundi i eines flotants | `Mapa_Mundi_Requisitos.md` i `Ventanas_y_Utilidades.md` |
| Revisar arquitectura de dades | `Arquitectura_Datos_Onegai.md` |
| Auditar camps i referències JSON | `Auditoria_Campos_Referencia.md` |
| Revisar constructor d'aventures per actes | `Plan_Implementacion_Constructor_Aventuras.md` |
| Revisar backlog, gantt i fases de feina | `Backlog_Tareas_y_Gantt.md` i `Gantt_Proyecto_ONEGAI.html` |
| Revisar el lot de prova Itx | `Lote_Itx_Guia_de_Revision.md` |

## Norma de manteniment

Quan afegeixis una funció que canviï fluxos, dades o pantalles:

1. Actualitza `Documentacio_Codi.md` si afecta arquitectura o rutes.
2. Actualitza el GDD o pla corresponent si afecta regles de joc.
3. Actualitza `../scripts/README.md` si apareix un script nou.
4. Actualitza `../data/README.md` si apareix una carpeta o contracte JSON nou.
5. Mantén la documentació i els comentaris en català; els identificadors nous de codi han d'anar en anglès.
