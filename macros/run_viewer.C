void run_viewer()
{
    auto run = new LKRun();
    run -> AddPar("config.mac");
    run -> AddInputFile("atomx_0013.D1.conv.root");
    run -> Add(new LKGETChannelViewer);

    run -> Init();
    run -> ExecuteNextEvent();
}
