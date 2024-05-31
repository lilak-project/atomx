void next(int eventID=-1) {
    if (eventID<0) LKRun::GetRun() -> ExecuteNextEvent();
    else LKRun::GetRun() -> ExecuteEvent(eventID);
}

void run_eve()
{
    auto run = new LKRun();
    run -> AddDetector(new AToMX());
    run -> AddPar("config_eve.mac");
    run -> Add(new LKPulseShapeAnalysisTask);
    run -> Add(new LKHTTrackingTask);
    run -> Add(new LKEveTask);
    run -> Init();

    next(1);
}
