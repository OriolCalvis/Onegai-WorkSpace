package cat.dnd.cc.eines;

import cat.dnd.cc.service.SistemaHabilidades;

@FunctionalInterface
public interface OtrosHandler {
    boolean test(String token, SistemaHabilidades.Contexto contexto, EngineConfig config);
}
