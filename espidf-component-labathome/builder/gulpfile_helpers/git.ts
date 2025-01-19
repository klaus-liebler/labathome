
import { execSync } from 'node:child_process';

const splitCharacter = '<##>'
const prettyFormat = ["%h", "%H", "%s", "%f", "%b", "%at", "%ct", "%an", "%ae", "%cn", "%ce", "%N", ""]



export async function getLastCommit(suppressStdOut: boolean) {
    const cmd = `git log -1 --pretty=format:"${prettyFormat.join(splitCharacter)}" && git rev-parse --abbrev-ref HEAD && git tag --contains HEAD`


    console.info(`Executing ${cmd}`)
    const stdout = execSync(cmd, {
        env: process.env
    }).toString();
    
    if (stdout === '') {
        throw new Error(`this does not look like a git repo`)
    }

    if (!suppressStdOut && stdout)
        console.log(stdout.toString());

    var a = stdout.split(splitCharacter)

    // e.g. master\n or master\nv1.1\n or master\nv1.1\nv1.2\n
    var branchAndTags = a[a.length - 1].split('\n').filter(n => n)
    var branch = branchAndTags[0]
    var tags = branchAndTags.slice(1)

    return {
        shortHash: a[0],
        hash: a[1],
        subject: a[2],
        sanitizedSubject: a[3],
        body: a[4],
        authoredOn: a[5],
        committedOn: a[6],
        author: {
            name: a[7],
            email: a[8],
        },
        committer: {
            name: a[9],
            email: a[10]
        },
        notes: a[11],
        branch,
        tags
    }
}
