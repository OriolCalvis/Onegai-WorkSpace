package cat.dnd.cc.config;

import jakarta.servlet.FilterChain;
import jakarta.servlet.ServletException;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import jakarta.servlet.http.HttpSession;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Component;
import org.springframework.web.filter.OncePerRequestFilter;

import java.io.IOException;
import java.time.Instant;

/**
 * Registra CADA petición HTTP: método, ruta, estado, duración y sesión.
 * - DEBUG: todas (la traza completa de la run).
 * - WARN: peticiones lentas (≥ umbral) o con estado 4xx/5xx, con el porqué visible.
 * El id de sesión (recortado) permite verificar que la sesión NO se pierde entre
 * peticiones: si el sufijo cambia sin haber cerrado el navegador, algo va mal.
 */
@Component
public class RegistroPeticionesFilter extends OncePerRequestFilter {

    private static final Logger log = LoggerFactory.getLogger(RegistroPeticionesFilter.class);

    private final MetricasPeticiones metricas;

    public RegistroPeticionesFilter(MetricasPeticiones metricas) {
        this.metricas = metricas;
    }

    @Override
    protected boolean shouldNotFilter(HttpServletRequest request) {
        String ruta = request.getRequestURI();
        // recursos estáticos: no aportan a la traza y ensucian el log
        return ruta.startsWith("/css/") || ruta.startsWith("/js/") || ruta.startsWith("/img/")
                || ruta.endsWith(".ico");
    }

    @Override
    protected void doFilterInternal(HttpServletRequest request, HttpServletResponse response,
                                    FilterChain chain) throws ServletException, IOException {
        long inicio = System.nanoTime();
        Throwable excepcion = null;
        try {
            chain.doFilter(request, response);
        } catch (ServletException | IOException | RuntimeException e) {
            excepcion = e;
            throw e;
        } finally {
            long ms = (System.nanoTime() - inicio) / 1_000_000;
            HttpSession sesion = request.getSession(false);
            String idSesion = sesion == null ? "sin-sesion" : abreviar(sesion.getId());
            // Si la petición murió por excepción, el status del response aún dice 200:
            // el contenedor lo cambia DESPUÉS. Aquí contamos la verdad: 500.
            int status = excepcion != null ? 500 : response.getStatus();
            String linea = String.format("%s %s -> %d en %d ms [sesion %s]%s",
                    request.getMethod(), request.getRequestURI(), status, ms, idSesion,
                    excepcion != null ? " — excepción: " + excepcion.getClass().getSimpleName()
                            + ": " + excepcion.getMessage() : "");

            metricas.registrar(new MetricasPeticiones.Registro(
                    Instant.now(), request.getMethod(), request.getRequestURI(), status, ms, idSesion));

            if (status >= 500) {
                log.error("PETICION FALLIDA: {}", linea);
            } else if (status >= 400) {
                log.warn("PETICION RECHAZADA (no critico): {}", linea);
            } else if (ms >= MetricasPeticiones.UMBRAL_LENTA_MS) {
                log.warn("PETICION LENTA (≥{} ms): {}", MetricasPeticiones.UMBRAL_LENTA_MS, linea);
            } else {
                log.debug("{}", linea);
            }
        }
    }

    private String abreviar(String id) {
        return id == null || id.length() <= 8 ? String.valueOf(id) : "…" + id.substring(id.length() - 8);
    }
}
