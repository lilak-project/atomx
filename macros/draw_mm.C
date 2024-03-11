void draw_mm()
{
    auto mm = new ATMicromegas();
    mm -> AddPar("config_eve.mac");
    mm -> Init();
    mm -> Draw("caac:colztext");
    //mm -> Draw("cobo:colztext");
    //mm -> Draw("asad:colztext");
    //mm -> Draw("aget:colztext");
    //mm -> Draw("chan:colztext");
    //mm -> Draw("layer:colztext");
    //mm -> Draw("row:colztext");
}
