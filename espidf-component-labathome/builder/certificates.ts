import * as forge from 'node-forge';
import * as fs from 'fs';
import * as crypto from "node:crypto";
import { DEFAULT_COUNTRY, DEFAULT_LOCALITY, DEFAULT_ORGANIZATION, DEFAULT_STATE, ROOT_CA_COMMON_NAME } from './gulpfile_config';


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



function createHostExtensions(dnsHostname: string, authorityKeyIdentifier: string) {
	return [{
		name: 'basicConstraints',
		cA: false
	},{
		name: 'subjectKeyIdentifier'
	}, {
		name: 'authorityKeyIdentifier',
		authorityCertIssuer: true,
		serialNumber: authorityKeyIdentifier
	}, {
		name: 'keyUsage',
		digitalSignature: true,
		nonRepudiation: true,
		keyEncipherment: true
	}, {
		name: 'extKeyUsage',
		serverAuth: true,
		clientAuth: true,
	}, {
		name: 'subjectAltName',
		altNames: [{
			type: 2, // 2 is DNS type
			value: dnsHostname
		}]
	}];
}

function createClientExtensions(username: string, authorityKeyIdentifier: string) {
	return [{
		name: 'basicConstraints',
		cA: false
	}, {
		name: 'keyUsage',
		digitalSignature: true,
		keyEncipherment: true
	}, {
		name: 'extKeyUsage',
		clientAuth: true,
	}];
}

function createSubject(commonName: string): forge.pki.CertificateField[] {
	return [{
		shortName: 'C',
		value: DEFAULT_COUNTRY
	}, {
		shortName: 'ST',
		value: DEFAULT_STATE
	}, {
		shortName: 'L',
		value: DEFAULT_LOCALITY
	}, {
		shortName: 'O',
		value: DEFAULT_ORGANIZATION//hier muss vermutlich was stehen
	}, 
	{
		shortName: 'CN',
		value: commonName//hier muss vermutlich beim host certificate etwas anderes stehen, als der Hostname
	}];
}

// a hexString is considered negative if it's most significant bit is 1
// because serial numbers use ones' complement notation
// this RFC in section 4.1.2.2 requires serial numbers to be positive
// http://www.ietf.org/rfc/rfc5280.txt
function randomSerialNumber(numberOfBytes: number) {
	let buf = crypto.randomBytes(numberOfBytes);
	buf[0] = buf[0] & 0x7F;
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
	cert.validity.notBefore = new Date();
	cert.validity.notAfter = DateNDaysInFuture(100 * 365);//validity 100 years from now
	cert.setSubject(subject);
	cert.setIssuer(issuer);
	cert.setExtensions(exts);
	cert.sign(signWith ?? keypair.privateKey, forge.md.sha512.create());
	return { certificate: forge.pki.certificateToPem(cert), privateKey: forge.pki.privateKeyToPem(keypair.privateKey), };
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
		createSubject(commonName), //CN of subject may not contain server hostname (found out by experiments)
		caCert.subject.attributes, //issuer is the subject of the rootCA
		createHostExtensions(dnsHostname, caCert.serialNumber),
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
		createClientExtensions(username, caCert.serialNumber),
		caPrivateKey //sign with private key of rootCA
	);
}

