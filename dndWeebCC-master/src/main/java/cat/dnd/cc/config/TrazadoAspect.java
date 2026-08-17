package cat.dnd.cc.config;

import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.stream.Collectors;

/**
 * Trazado transversal de la capa de negocio, sin tocar ninguna clase:
 *
 *  - Servicios (cat.dnd.cc.service.*): DEBUG con método, argumentos y duración.
 *    Si un método tarda de más, WARN con el tiempo (rendimiento visible).
 *  - Repositorios, solo escrituras (save* / delete*): INFO — quede constancia de
 *    QUÉ se guardó/borró y cuándo, para verificar que la información no se pierde.
 *  - Cualquier excepción en ambas capas: se registra con su porqué antes de
 *    propagarse. Las IllegalArgumentException (validaciones, ids inexistentes)
 *    son WARN (error no crítico); el resto, ERROR.
 */
@Aspect
@Component
public class TrazadoAspect {

    private static final Logger log = LoggerFactory.getLogger("cat.dnd.cc.trazado");
    private static final long UMBRAL_SERVICIO_LENTO_MS = 250;

    @Around("execution(public * cat.dnd.cc.service..*(..))")
    public Object trazarServicio(ProceedingJoinPoint punto) throws Throwable {
        return trazar(punto, false);
    }

    @Around("execution(public * cat.dnd.cc.repository..save*(..)) || "
            + "execution(public * cat.dnd.cc.repository..delete*(..))")
    public Object trazarEscritura(ProceedingJoinPoint punto) throws Throwable {
        return trazar(punto, true);
    }

    private Object trazar(ProceedingJoinPoint punto, boolean esEscritura) throws Throwable {
        String firma = punto.getSignature().getDeclaringType().getSimpleName()
                + "." + punto.getSignature().getName();
        String args = resumenArgs(punto.getArgs());
        long inicio = System.nanoTime();
        try {
            Object resultado = punto.proceed();
            long ms = (System.nanoTime() - inicio) / 1_000_000;
            if (esEscritura) {
                log.info("GUARDADO OK: {}({}) en {} ms", firma, args, ms);
            } else if (ms >= UMBRAL_SERVICIO_LENTO_MS) {
                log.warn("SERVICIO LENTO (≥{} ms): {}({}) tardó {} ms",
                        UMBRAL_SERVICIO_LENTO_MS, firma, args, ms);
            } else {
                log.debug("{}({}) OK en {} ms", firma, args, ms);
            }
            return resultado;
        } catch (IllegalArgumentException e) {
            long ms = (System.nanoTime() - inicio) / 1_000_000;
            log.warn("NO CRITICO en {}({}) tras {} ms — porqué: {}", firma, args, ms, e.getMessage());
            throw e;
        } catch (Throwable e) {
            long ms = (System.nanoTime() - inicio) / 1_000_000;
            log.error("FALLO en {}({}) tras {} ms — porqué: {}: {}",
                    firma, args, ms, e.getClass().getSimpleName(), e.getMessage(), e);
            throw e;
        }
    }

    /** Argumentos abreviados: suficientes para saber sobre QUÉ se operó, sin volcar objetos enteros. */
    private String resumenArgs(Object[] args) {
        if (args == null || args.length == 0) {
            return "";
        }
        return Arrays.stream(args)
                .map(a -> {
                    if (a == null) return "null";
                    String s = String.valueOf(a);
                    if (a instanceof java.util.Collection<?> c) return "[" + c.size() + " elems]";
                    return s.length() > 60 ? s.substring(0, 57) + "..." : s;
                })
                .collect(Collectors.joining(", "));
    }
}
