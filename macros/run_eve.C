void next(int eventID=-1)
{
    if (eventID<0)
        LKRun::GetRun() -> ExecuteNextEvent();
    else
        LKRun::GetRun() -> ExecuteEvent(eventID);
}

void run_eve()
{
    auto run = new LKRun();
    run -> SetTag("eve");
    run -> AddPar("config_eve.mac");
    run -> AddInputFile("/home/ejungwoo/data/atomx/atomx_0058.D0.conv.root");
    run -> AddDetector(new AToMX());
    //run -> Add(new LKGETChannelViewer);
    run -> Add(new LKEveTask);
    run -> Init();
    //run -> Print();
    next(109);
}
