export enum ControllerState {
    CREATED,
    STARTED,
    STOPPED,
}

export abstract class ScreenController {
    private state: ControllerState;
    constructor(protected div: HTMLDivElement) {
        this.hideDIV();
        this.state = ControllerState.CREATED;
    }
    get ElementId() { return this.div.id; }
    get State() { return this.state; }
    set State(value: ControllerState) { this.state = value; }
    abstract onCreate(): void;
    abstract onFirstStart(): void;
    abstract onRestart(): void;
    abstract onStop(): void;
    public showDIV() {
        this.div.style.display = "block";
    }
    public hideDIV() {
        this.div.style.display = "none";
    }
}
