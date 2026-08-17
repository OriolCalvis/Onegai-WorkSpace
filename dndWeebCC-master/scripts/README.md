# Scripts de contingut i manteniment

Els scripts modifiquen o auditen el catàleg JSON. Molts poden crear centenars de fitxers: fes commit abans d'executar-los.

## Rutina segura

```bash
git status --short
bash scripts/desar.sh "chore: punt segur abans de generar contingut"
python3 scripts/generar_x.py --dry-run
python3 scripts/generar_x.py
python3 scripts/auditar_campos_referencia.py
```

## Inventari

| Script | Funció |
|---|---|
| `auditar_campos_referencia.py` | Detecta camps que semblen ids/referències i possibles trencaments |
| `desar.sh` | Commit local amb missatge passat per argument |
| `exportar_catalogo_ids.py` | Exporta ids reals per prompts i revisió |
| `generar_artefactos.py` | Genera artefactes/equip especial |
| `generar_enemigos.py` | Genera enemics regulars |
| `generar_equipo_data.py` / `generar_equipo_data_2.py` | Genera equip |
| `generar_canonicos_ed2.py` | Genera o normalitza contingut canònic ED2 compatible amb CON/DES/INT/CAR |
| `generar_habilidades_clases.py` | Genera habilitats per classe/tier amb dry-run |
| `generar_historias_npcs.py` | Genera històries i PNJs vinculats |
| `generar_invocaciones_trampas_monturas.py` | Genera invocacions, trampes i montures |
| `generar_jefes.py` | Genera caps amb fases, arena i accions llegendàries |
| `generar_loot.py` | Genera taules de botí |
| `generar_lote_itx.py` | Crea o neteja el lot Itx de prova |
| `generar_panteon.py` | Genera deitats/panteó |
| `generar_trasfondos_v2.py` | Genera trasfons de mesos |
| `migrar_a_tiers.py` | Migració de contingut antic al sistema de tiers |
| `prueba_rendimiento.sh` | Prova ràpida de rendiment |
| `renombrar_canonicos_es_en.py` | Reanomena o harmonitza fitxers canònics entre nomenclatures ES/EN |

## Contractes

- Els scripts han de ser idempotents quan sigui possible: no sobreescriure sense avisar.
- Preferir `--dry-run` en scripts nous.
- Escriure JSON amb `ensure_ascii=False` i indent estable.
- Els ids han de ser `snake_case` i no inventar referències que no existeixin.
