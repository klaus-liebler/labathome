import proc from "node:child_process";
import * as util from "node:util"

const splitCharacter = '<##>'
const prettyFormat = ["%h", "%H", "%s", "%f", "%b", "%at", "%ct", "%an", "%ae", "%cn", "%ce", "%N", ""]



export async function getLastCommit() {
    const command = `git log -1 --pretty=format:"${prettyFormat.join(splitCharacter)}" && git rev-parse --abbrev-ref HEAD && git tag --contains HEAD`
    const execPromise = util.promisify(proc.exec);
    
    const { stdout, stderr } = await execPromise(command, {cwd: __dirname, env: process.env});
    if (stdout === '') {
        throw new Error(`this does not look like a git repo`)
    }

    if (stderr) {
        throw new Error(stderr);
    }
    console.log(stdout)
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

module.exports = {
    getLastCommit
}
