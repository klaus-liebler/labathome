import * as forge from 'node-forge';
import * as fs from 'fs';
import * as crypto from "node:crypto";
import { DEFAULT_COUNTRY, DEFAULT_LOCALITY, DEFAULT_ORGANIZATION, DEFAULT_STATE, ROOT_CA_COMMON_NAME } from '../gulpfile_config';


function createRootCaExtensions() {
	return [{
		name: 'basicConstraints',
		cA: true
	}, {
		name: 'keyUsage',
		keyCertSign: true,
		cRLSign: true
	}];
}



function createUniversalAuthExtensions(dnsHostname: string, authorityKeyIdentifier: string) {
	return [
		{
			name: 'authorityKeyIdentifier',
			authorityCertIssuer: true,
			serialNumber: authorityKeyIdentifier
		},{
			name: 'basicConstraints',
			cA: false
		},  {
			name: 'keyUsage',
			digitalSignature: true,
			nonRepudiation: true,
			keyEncipherment: true,
			dataEncipherment: true,
		},{
			name: 'extKeyUsage',
			serverAuth: true,
			clientAuth: true,
		},{
			name: 'subjectAltName',
			altNames: [{
				type: 2, // 2 is DNS type
				value: dnsHostname
				}
			]
		},{
			name: 'subjectKeyIdentifier'
		}
	];
}


function createSubject(commonName: string): forge.pki.CertificateField[] {
	return [{
		shortName: 'C',
		value: DEFAULT_COUNTRY,
		
	}, {
		shortName: 'ST',
		value: DEFAULT_STATE,
		valueTagClass: forge.asn1.Type.UTF8
	}, {
		shortName: 'L',
		value: DEFAULT_LOCALITY,
		valueTagClass: forge.asn1.Type.UTF8
	}, {
		shortName: 'O',
		value: DEFAULT_ORGANIZATION,//hier muss vermutlich was stehen
		valueTagClass: forge.asn1.Type.UTF8
	}, 
	{
		shortName: 'CN',
		value: commonName,//hier muss vermutlich beim host certificate etwas anderes stehen, als der Hostname
		valueTagClass: forge.asn1.Type.UTF8
	}];
}

// a hexString is considered negative if it's most significant bit is 1
// because serial numbers use ones' complement notation
// this RFC in section 4.1.2.2 requires serial numbers to be positive
// http://www.ietf.org/rfc/rfc5280.txt
function randomSerialNumber(numberOfBytes: number) {
	let buf = crypto.randomBytes(numberOfBytes);
	buf[0] = buf[0] & 0x7F | 0x20;
	return buf.toString("hex");
}

function DateNDaysInFuture(n: number) {
	var d = new Date();
	d.setDate(d.getDate() + n);
	return d;
}

function certHelper(setPrivateKeyInCertificate: boolean, subject: forge.pki.CertificateField[], issuer: forge.pki.CertificateField[], exts: any[], signWith: forge.pki.PrivateKey | null) {
	const keypair = forge.pki.rsa.generateKeyPair(2048);
	const cert = forge.pki.createCertificate();
	cert.publicKey = keypair.publicKey;
	if (setPrivateKeyInCertificate) cert.privateKey = keypair.privateKey;
	cert.serialNumber = randomSerialNumber(20);
	cert.validity.notBefore = DateNDaysInFuture(-1);
	cert.validity.notAfter = DateNDaysInFuture(3000);//8 Years
	cert.setSubject(subject);
	cert.setIssuer(issuer);
	cert.setExtensions(exts);
	cert.sign(signWith ?? keypair.privateKey, forge.md.sha256.create());
	return { certificate: forge.pki.certificateToPem(cert), privateKey: forge.pki.privateKeyToPem(keypair.privateKey), };
}
export function CreateAndSignCertWithGivenPublicKey(publicKeyPemPath: fs.PathOrFileDescriptor, commonName:string, dnsHostname: string, certificateCaPemPath: fs.PathOrFileDescriptor, caPrivateKeyPemPath:fs.PathOrFileDescriptor) {

	let caCert = forge.pki.certificateFromPem(fs.readFileSync(certificateCaPemPath).toString());
	let caPrivateKey = forge.pki.privateKeyFromPem(fs.readFileSync(caPrivateKeyPemPath).toString());
	let publicKey = forge.pki.publicKeyFromPem(fs.readFileSync(publicKeyPemPath).toString());
	const cert = forge.pki.createCertificate();
	cert.publicKey = publicKey;
	cert.serialNumber = randomSerialNumber(20);
	cert.validity.notBefore = DateNDaysInFuture(-1);//8 Years
	cert.validity.notAfter = DateNDaysInFuture(3000);//8 Years
	cert.setSubject(createSubject(commonName));
	cert.setIssuer(caCert.subject.attributes); //issuer is the subject of the rootCA);
	cert.setExtensions(createUniversalAuthExtensions(dnsHostname, caCert.serialNumber));
	cert.sign(caPrivateKey, forge.md.sha256.create());
	return forge.pki.certificateToPem(cert);
}

export function CreateRootCA() {
	const subjectAndIssuer = createSubject(ROOT_CA_COMMON_NAME);
	return certHelper(
		true,//necessary, found out in tests
		subjectAndIssuer,
		subjectAndIssuer, //self sign
		createRootCaExtensions(),
		null);//self sign
}

export function CreateAndSignCert(commonName:string, dnsHostname: string, certificateCaPemPath: fs.PathOrFileDescriptor, privateKeyCaPemPath: fs.PathOrFileDescriptor) {
	let caCert = forge.pki.certificateFromPem(fs.readFileSync(certificateCaPemPath).toString());
	let caPrivateKey = forge.pki.privateKeyFromPem(fs.readFileSync(privateKeyCaPemPath).toString());
	return certHelper(
		false,
		createSubject(commonName),
		caCert.subject.attributes, //issuer is the subject of the rootCA
		createUniversalAuthExtensions(dnsHostname, caCert.serialNumber),
		caPrivateKey //sign with private key of rootCA
	);
}

export function CreateAndSignClientCert(username: string, certificateCaPemPath: fs.PathOrFileDescriptor, privateKeyCaPemPath: fs.PathOrFileDescriptor) {
	let caCert = forge.pki.certificateFromPem(fs.readFileSync(certificateCaPemPath).toString());
	let caPrivateKey = forge.pki.privateKeyFromPem(fs.readFileSync(privateKeyCaPemPath).toString());
	return certHelper(
		false,
		createSubject(username),
		caCert.subject.attributes, //issuer is the subject of the rootCA
		createUniversalAuthExtensions(username, caCert.serialNumber),
		caPrivateKey //sign with private key of rootCA
	);
}

