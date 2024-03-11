void run_psa()
{
    auto run = new LKRun();
    run -> AddPar("config_psa.mac");
    run -> AddInputFile("/home/ejungwoo/data/atomx/atomx_0058.D0.conv.root");
    run -> AddDetector(new AToMX());
    run -> Add(new LKPulseShapeAnalysisTask);
    run -> Init();
    run -> Run();
}
