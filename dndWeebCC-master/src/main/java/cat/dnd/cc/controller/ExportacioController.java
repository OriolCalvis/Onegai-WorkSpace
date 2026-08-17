package cat.dnd.cc.controller;

import cat.dnd.cc.model.Exportacio;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;

import java.util.List;

@Controller
@RequestMapping("/exportacions")
public class ExportacioController {
    @GetMapping
    public String index(Model model) {
        model.addAttribute("currentPage", "exportacions");
        model.addAttribute("exportacions", List.of(
                new Exportacio("JSON de personatge", "/personatges/{id}/export/json", "application/json"),
                new Exportacio("PDF de personatge", "/personatges/{id}/export/pdf", "application/pdf")
        ));
        return "exportacions/index";
    }
}
