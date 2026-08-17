package cat.dnd.cc.config;

import org.springframework.stereotype.Component;

import java.time.Instant;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Deque;
import java.util.List;
import java.util.concurrent.atomic.AtomicLong;

/**
 * Memoria viva de la run: las últimas peticiones con su duración y estado, más
 * contadores globales. La llena RegistroPeticionesFilter y la lee el panel
 * /diagnostico — así "ejecutar una run y ver qué funciona y cómo" es literal.
 */
@Component
public class MetricasPeticiones {

    /** Umbral a partir del cual una petición se considera lenta (y se avisa en WARN). */
    public static final long UMBRAL_LENTA_MS = 500;

    public record Registro(Instant instante, String metodo, String ruta, int status,
                           long duracionMs, String sesion) {
        public boolean esLenta() { return duracionMs >= UMBRAL_LENTA_MS; }
        public boolean esError() { return status >= 400; }
    }

    private static final int CAPACIDAD = 200;

    private final Deque<Registro> ultimas = new ArrayDeque<>(CAPACIDAD);
    private final AtomicLong total = new AtomicLong();
    private final AtomicLong errores = new AtomicLong();
    private final AtomicLong lentas = new AtomicLong();
    private final Instant arranque = Instant.now();

    public synchronized void registrar(Registro registro) {
        if (ultimas.size() >= CAPACIDAD) {
            ultimas.removeLast();
        }
        ultimas.addFirst(registro);
        total.incrementAndGet();
        if (registro.esError()) {
            errores.incrementAndGet();
        }
        if (registro.esLenta()) {
            lentas.incrementAndGet();
        }
    }

    public synchronized List<Registro> ultimas() {
        return new ArrayList<>(ultimas);
    }

    public long total() { return total.get(); }
    public long errores() { return errores.get(); }
    public long lentas() { return lentas.get(); }
    public Instant arranque() { return arranque; }

    /** Duración media (ms) de las peticiones retenidas en memoria. */
    public synchronized long mediaMs() {
        return ultimas.isEmpty() ? 0
                : (long) ultimas.stream().mapToLong(Registro::duracionMs).average().orElse(0);
    }
}
