package com.onegai;

import java.util.List;

/**
 * Compatibility facade for legacy code that still imports {@code com.onegai.SistemaHabilidades}.
 * The implementation lives in {@link cat.dnd.cc.service.SistemaHabilidades}.
 */
public class SistemaHabilidades extends cat.dnd.cc.service.SistemaHabilidades {
    public SistemaHabilidades(int fuerza, int destreza, int constitucion,
                              int inteligencia, int sabiduria, int carisma,
                              int nivel, List<String> competencias) {
        super(fuerza, destreza, constitucion, inteligencia, sabiduria, carisma, nivel, competencias);
    }

    @Override
    public SistemaHabilidades setPuedeLanzarHechizos(boolean puedeLanzarHechizos) {
        super.setPuedeLanzarHechizos(puedeLanzarHechizos);
        return this;
    }

    @Override
    public SistemaHabilidades addEtiqueta(String etiqueta) {
        super.addEtiqueta(etiqueta);
        return this;
    }

    @Override
    public SistemaHabilidades conocerPacto(String nombre) {
        super.conocerPacto(nombre);
        return this;
    }

    @Override
    public SistemaHabilidades setStrictOtrosChecks(boolean strictOtrosChecks) {
        super.setStrictOtrosChecks(strictOtrosChecks);
        return this;
    }
}
