# D-10 — Validación de catálogos reals: `validar_todo.py` comprova
# referències creuades, IDs i camp mecànics de tots els 24 JSONs
# d'`assets/catalogs/'. Si una entrada perd un camp respecte de la
# línia base, el test falla.
#
# Fase 1 (connectivity) es saltada perquè té desembarco_landmass_*
# already aplazats al TABLERO. Les fases 2-5 validen el contingut
# real dels catàlegs.
#
# És el bridge entre la validació Python rica (que ja existeix) i CI:
# sense això, les 5 fases de `validar_todo.py` només les corre qui
# executa manualment l'script.
include(CTest)
find_program(MOTOR_PYTHON3 python3)
if(BUILD_TESTING AND MOTOR_PYTHON3)
    add_test(NAME catalog_content_validation
        COMMAND ${MOTOR_PYTHON3}
            ${CMAKE_CURRENT_SOURCE_DIR}/tools/validar_todo.py
            --skip-phase1
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    set_tests_properties(catalog_content_validation PROPERTIES
        LABELS "unit;gl-free"
        TIMEOUT 60)
endif ()
