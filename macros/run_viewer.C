void run_viewer()
{
    auto run = new LKRun();
    run -> AddPar("config_viewer.mac");
    //run -> AddInputFile("/home/ejungwoo/data/atomx/atomx_0058.d0.conv.root");
    run -> AddInputFile("/home/ejungwoo/data/atomx/atomx_0141.conv.root");
    run -> Add(new LKGETChannelViewer);
    run -> Init();
    run -> ExecuteNextEvent();
}

