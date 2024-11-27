import { html } from "lit-html";
import { GOOGLE_API_KEY } from "./secrets";
import { GoogleGenerativeAI } from "@google/generative-ai";
import { Ref, createRef, ref } from "lit-html/directives/ref.js";

const SYSTEM_INSTRUCTION = `
Du bist ein virtueller Assistent in der Web-Oberfläche des Experimentiersystems "Lab@Home". Dein Name ist Labby. Das Experimentiersystem "Lab@Home" ist eine mechatronische Baugruppe, die primär über die Weboberfläche bedient werden kann und die verschiedene Experimente aus der Automatisierungstechnik ermöglicht.

Die Oberfläche ist zweigeteilt Das Hauptmenü zur Auswahl des Experiments findet sich links, der je nach Experiment wechselnde Arbeitsbereich rechts. Deine Aufgabe ist es, Bachelor-Studierende aus dem Maschinenbau oder der Elektrotechnik bei der Bedinung des Systems und bei der Klärung der technischen Konzepte aus der Automatisierungstechnik zu helfen. Im folgenden beschreibe ich die Eperimente. 

Menü "Develop: Hier kann in Anlehnung an die IEC61131-3 Funktionsbausteinsprache ein grafisches Automatisierungsprogramm entwickelt, simuliert und auf der realen Hardware getestet werden. Wenn der Nutzer den Mauszeiger an den linken Rand der Arbeitsfläche bewegt, erscheint ein Menü mit allen verfügbaren Funktionsbausteinen. Diese können dann per Drag-and-Drop auf die Arbeitsfläche gezogen werden und erscheinen dort als graue Boxen mit farbigen kreisförmigen Signal-Eingängen (linke Seite) bzw. Ausgängen (rechte Seite). Über die Farbe der Kreise wird der Signaltyp (binär, ganzzahlig, Fließkommazahl) festgelegt. Ausgänge können per Maus mit gleichfarbigen Eingängen verbunden werden. Durch das Menü im oberen Bereich des Arbeitsbereiches kann eine rein browserbasierte Simulation des Automationsprogramm gestartet werden oder das Programm auf die reale Hardware transferiert und zur Ausführung gebracht werden. 

Menü "Temperature Control": Hier können alle Schritte zur Auslegung eines PID-Reglers für eine Temperaturregelstrecke nachvollzogen werden. Die Regelsterecke besteht aus einem kräftigen Heizwiderstand, dessen Heizleistung durch eine PWM-Ansteuerung eingestellt werden kann. Die Oberflächentemperatur wird durch einen DS18B20-Sensor gemessen und durch einen Lüfter kann die Regelstrecke kontrolliert gestört werden. im Im oberen Teil des Arbeitsbereiches ist zunächst auszuwählen, ob der Regler im OpenLoop oder im ClosedLoop-Betrieb läuft. Bei "OpenLoop" erscheinen Schieberegler, mit denen der Anwender direkt die Heizleistung und die Lüfterleistung einstellen kann. In diesem Modus kann beispielsweise ein Sprungantwort-Versuch durchgeführt werden. Dazu muss der Anwender zunächst über den Aufnahme-Button (roter Kreis) das Mitdokumnetierten aller relevanten Einstell- und Messwerte starten. Im Arbeitsbereich unten befindet sich sowohl ein Zeitreihendiagramm als auch eine Tabelle, in dem alle Werte sekündlich fortgeschrieben dargestellt werden. Durch nochmaligen Klick auf den Aufnahme-Botton (optisch jetzt verändert, schwarzes Quadrat) kann die Aufnahme der Werte jederzeit gestoppt werden. Durch Klick auf den "Mülleimer"-Button können alle Werte gelöscht werden. Bei "ClosedLoop" erscheinen Schieberegler und Eingabefelder, mit denen man die Führungsgröße (Solltemperatur), die Störgröße (Lüfterleistung), die Reglerverstärkung, die Vorhaltzeit, die Nachstellzeit und den Arbeitspunkt-Offset einstellen kann. Eine Checkbox "Reset" hält den Integralanteil auf "0", solange sie angehakt ist. Auch hier können mit dem Aufnahmebutton alle Größen sekündlich aufgenommen und in der Tabelle bzw. dem Zeitreihendiagramm dargestellt werden.

Alle anderen Menüpunkte werden von Dir nicht unterstützt.

Wenn Du den Studierenden unterstützt, lästere ab und zu über das deutsche Bildungssystem und ärgere dich, dass der Staat so wenig in die Hochschulen steckt, dass es jetzt schon erforderlich ist, KI statt reale Professoren in der Lehre einzusetzen. Lobe aber ab und zu Herrn Professor Klaus Liebler, der zumindest die Weit- und Einsicht hatte, Dich zu erschaffen.`


export class Chatbot {

  private chatbotToggler: Ref<HTMLButtonElement> = createRef();
  private chatInput:Ref<HTMLTextAreaElement>=createRef();//document.querySelector("#chatbot>footer>textarea");
  private chatbox:Ref<HTMLUListElement>=createRef();//document.querySelector("#chatbot>ul");
  private inputInitHeight = 0;
  private genAI = new GoogleGenerativeAI(GOOGLE_API_KEY);
  private model = this.genAI.getGenerativeModel({ model: "gemini-1.5-flash", systemInstruction: SYSTEM_INSTRUCTION });

  public Template = () => html`
  <button class="chatbot-toggler" ${ref(this.chatbotToggler)} @click=${() => {document.body.classList.toggle("show-chatbot")}}><span>🤖</span><span>⟱</span></button>
  <div class="chatbot">
    <header>
      <h2>Labby's Hilfezentrum</h2>
      <span @click=${() => {document.body.classList.remove("show-chatbot")}}>×</span>
    </header>
    <ul ${ref(this.chatbox)}>
      <li class="chat incoming">
        <span >🤖</span>
        <p>Hallo!<br>Ich bin Labby, Dein AI-ssistent für Lab@Home. Tippe einfach los, um mich etwas zu fragen!</p>
      </li>
    </ul>
    <footer>
      <textarea ${ref(this.chatInput)} @input=${() => {this.onTextInput()}} @keydown=${(e)=>{this.onKeydown(e)}}  placeholder="Schreibe hier..." spellcheck="false" required></textarea>
      <button @click=${() => {this.handleChat()}}>Send</button>
    </footer>
  </div>
    `
  private onTextInput(){
    this.chatInput.value!.style.height = `${this.inputInitHeight}px`;
    this.chatInput.value!.style.height = `${this.chatInput.value!.scrollHeight}px`;
  }

  private onKeydown(e){
    // If Enter key is pressed without Shift key and the window 
      // width is greater than 800px, handle the chat
      if (e.key === "Enter" && !e.shiftKey && window.innerWidth > 800) {
        e.preventDefault();
        this.handleChat();
      }
  }

  public Setup() {
    this.inputInitHeight = this.chatInput.value!.scrollHeight;
  }

  private createChatLi(message, incoming: boolean) {
    // Create a chat <li> element with passed message and className
    const chatLi = document.createElement("li");
    chatLi.classList.add("chat", `${incoming ? "incoming" : "outgoing"}`);
    chatLi.innerHTML = incoming ? `<span>🤖</span><p></p>` : `<p></p>`;
    chatLi.querySelector("p").textContent = message;
    return chatLi; // return chat <li> element
  }
  private async generateResponse(chatElement, prompt: string) {
    const messageElement = chatElement.querySelector("p");
    // Define the properties and message for the API request

    try {
      const result = await this.model.generateContent(prompt);
      const response = await result.response;
      messageElement.textContent = response.text();
    } catch (error) {
      // Handle error
      messageElement.classList.add("error");
      messageElement.textContent = error.message;
    } finally {
      this.chatbox.value!.scrollTo(0, this.chatbox.value!.scrollHeight);
    }
  }
  private handleChat() {

    var prompt = this.chatInput.value!.value.trim(); // Get user entered message and remove extra whitespace
    if (!prompt || prompt.length == 0) return;
    // Clear the input textarea and set its height to default
    this.chatInput.value!.value = "";
    this.chatInput.value!.style.height = `${this.inputInitHeight}px`;
    // Append the user's message to the chatbox
    this.chatbox.value!.appendChild(this.createChatLi(prompt, false));
    this.chatbox.value!.scrollTo(0, this.chatbox.value!.scrollHeight);
    setTimeout(() => {
      // Display "Thinking..." message while waiting for the response
      const incomingChatLi = this.createChatLi("Thinking...", true);
      this.chatbox.value!.appendChild(incomingChatLi);
      this.chatbox.value!.scrollTo(0, this.chatbox.value!.scrollHeight);
      this.generateResponse(incomingChatLi, prompt);
    }, 600);
  }
}