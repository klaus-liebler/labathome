import * as gulp from "gulp";
import tts from "@google-cloud/text-to-speech"
import { google } from "@google-cloud/text-to-speech/build/protos/protos";
import { existsBoardSpecificPath, IApplicationInfo, IBoardInfo, strInterpolator, writeBoardSpecificFileCreateDirLazy } from "./gulpfile_utils";
import * as P from "./paths";

class FilenameAndSsml{constructor(public name:string, public ssml:string){}}



  export async function listvoices(cb: gulp.TaskFunctionCallback){
    const languageCode="de";
    const client = new tts.TextToSpeechClient();
    const [result] = await client.listVoices({languageCode});
    const voices = result.voices as google.cloud.texttospeech.v1.IVoice[];
    voices.forEach((voice) => {
      console.log(`${voice.name} (${voice.ssmlGender}): ${voice.languageCodes}`);
    });
  }

  //Dieser TTS-Client benötigt eine System-Umgebungsvariable "GOOGLE_APPLICATION_CREDENTIALS", die auf den kompletten Pfad einer JSON-Schlüsseldatei verweist
//Der Inhalt dieser Datei lässt sich wie hier beschrieben erzeugen: https://codelabs.developers.google.com/codelabs/cloud-text-speech-node?hl=de#3
export async function createSpeech(bi: IBoardInfo){
  var boardSounds=bi.board_settings?.speech as Array<FilenameAndSsml>|undefined;
  var boardTypeSounds=bi.board_type_settings?.speech as Array<FilenameAndSsml>|undefined;
  var appSounds=bi.application_settings?.speech as Array<FilenameAndSsml>|undefined;
  var sounds=new Array<FilenameAndSsml>().concat(boardSounds??[], boardTypeSounds??[], appSounds??[]);
  
  const client = new tts.TextToSpeechClient();
  // Construct the request
  for (const e of sounds) {
    if (existsBoardSpecificPath(bi, P.SOUNDS_DE_SUBDIR, e.name+".mp3")){
      continue;
    }
    var expanded_ssml= strInterpolator(e.ssml, bi);
    console.log(`Fetching from Google TTS: ${expanded_ssml}`);
    const request:google.cloud.texttospeech.v1.ISynthesizeSpeechRequest = {
      input: { ssml: expanded_ssml },
      // Select the language and SSML voice gender (optional)
      voice: { name: 'de-DE-Neural2-F', languageCode:"de-DE"},
      // select the type of audio encoding
      audioConfig: { audioEncoding: google.cloud.texttospeech.v1.AudioEncoding.MP3, sampleRateHertz: 22050 },
    };
    const [response] = await client.synthesizeSpeech(request);
    // Write the binary audio content to a local file
    writeBoardSpecificFileCreateDirLazy(bi,P.SOUNDS_DE_SUBDIR, e.name+".mp3", response.audioContent as Uint8Array);
  };
}